package com.dev.yunfan.devtelemetry;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;
import java.util.UUID;


public class BtDataService extends Service {
    private static final String TAG = "DATA_SERVICE";
    private static BluetoothAdapter btAdapter;
    private static BluetoothSocket btSocket;
    private static final UUID MY_UUID = UUID.randomUUID();

    private Queue<DataObj> dataCache = new LinkedList<>();
    private String sessionName = "default";
    private DataObj mostUpToDate;
    private final Binder binder = new LocalBinder();

    public class LocalBinder extends Binder {
        BtDataService getService() {
            return BtDataService.this;
        }
    }


    public BtDataService() {
    }

    @Override
    public void onCreate() {
        Log.e(TAG, "is created");
        btAdapter = BluetoothAdapter.getDefaultAdapter();
        if (btAdapter == null) {
            Log.e(TAG, "Bluetooth not supported by this phone!");
        } else if (!btAdapter.isEnabled()) {
            Log.e(TAG, "Bluetooth is not enabled!");
        }
        connect();
        BtConnectionThread btComm = new BtConnectionThread();
        btComm.run();
        HttpRequestThread httpRequest = new HttpRequestThread();
        httpRequest.run();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        Log.e(TAG, "is started");
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        disconnect();
    }

    private void connect() {
        if (btAdapter.isDiscovering())
            btAdapter.cancelDiscovery();
        Set<BluetoothDevice> devices = btAdapter.getBondedDevices();
        BluetoothDevice btDevice = null;

        for (BluetoothDevice device : devices) {
            if (device.getAddress().equals("20:16:10:27:14:80")) {
                btDevice = device;
                break;
            }
        }

        if (btDevice == null) {
            Log.e(TAG, "Did not find device!");
            return;
        }
        try {
            btSocket = btDevice.createInsecureRfcommSocketToServiceRecord(MY_UUID);
        } catch (IOException e) {
            Log.e(TAG, "Cannot create socket");
        }

        try {
            btSocket.connect();
        } catch (IOException e) {
            Log.e(TAG, "Something happened during BT connect. Try again with reflection.", e);
            try {
                btSocket = (BluetoothSocket) btDevice.getClass().getMethod("createRfcommSocket", new Class[]{int.class}).invoke(btDevice, 1);
                btSocket.connect();
            } catch (IOException refE) {
                Log.e(TAG, "Reflection failed", refE);
                disconnect();
            } catch (Exception otherE) {
                Log.e(TAG, "Create Reflection failed", otherE);
                disconnect();
            }
        }
    }

    private void disconnect() {
        try {
            btSocket.close();
        } catch (IOException e) {
            Log.e(TAG, "cannot close socket!", e);
        }
    }

    public DataObj getMostRecent() {
        return mostUpToDate;
    }

    private class BtConnectionThread extends Thread {
        private InputStream inStream;
        private OutputStream outStream;

        private BtConnectionThread() {
            if (btSocket == null) {
                Log.e(TAG, "No socket established in BtConnectionThread constructor.");
                return;
            }
            // Get the BluetoothSocket input and output streams
            try {
                inStream = btSocket.getInputStream();
                outStream = btSocket.getOutputStream();
            } catch (IOException e) {
                Log.e(TAG, "Socket stream cannot be created", e);
            }
        }

        public void run() {
            if (btSocket == null) {
                Log.e(TAG, "No socket established in run.");
                return;
            }
            // Keep listening to the InputStream while connected
            while (btSocket.isConnected()) {
                byte[] buffer = new byte[4096];
                String result;
                try {
                    int bytesTpRead = inStream.read(buffer);
                    result = new String(buffer, 0, bytesTpRead, StandardCharsets.US_ASCII);
                    DataObj newData = new DataObj(result, sessionName);
                    dataCache.add(newData);
                    mostUpToDate = newData;
                } catch (IOException e) {
                    Log.e(TAG, "disconnected", e);
                    disconnect();
                    break;
                } catch (Exception e) {
                    Log.e(TAG, "Unknown Error during BT transmission", e);
                }
            }
        }

        public void write(String stringToWrite) {
            byte[] buffer = stringToWrite.getBytes(StandardCharsets.US_ASCII);
            try {
                outStream.write(buffer);
            } catch (IOException e) {
                Log.e(TAG, "Exception during write", e);
            }
        }
    }

    private class HttpRequestThread extends Thread {
        private URL url;
        private HttpURLConnection httpConnection;

        HttpRequestThread() {
            try {
                url = new URL("http://yunfan.colab.duke.edu:5000/upload/");
            } catch (java.net.MalformedURLException e) {
                Log.e(TAG, "URL is wrong!");
            }
        }

        @Override
        public void run() {
            while (!dataCache.isEmpty()) {
                DataObj dataObj = dataCache.peek();
                try {
                    sendRequest(dataObj);
                    Log.e(TAG, "HTTP POST Success!");
                    dataCache.remove(dataObj);
                } catch (Exception e) {
                    Log.e(TAG, "HTTP failed", e);
                }
                try {
                    sleep(1);
                } catch (InterruptedException e) {
                    Log.e(TAG, "Cannot go to sleep.");
                }
            }
        }

        private void sendRequest(DataObj obj) throws Exception {
            String body = obj.toString();
            httpConnection = (HttpURLConnection) url.openConnection();
            httpConnection.setRequestMethod("POST");
            httpConnection.setRequestProperty("Content-Type", "application/json");
            httpConnection.setConnectTimeout(1500);
            httpConnection.setReadTimeout(1000);
            httpConnection.setDoOutput(true);
            httpConnection.setDoInput(true);
            httpConnection.setRequestProperty("Content-Length", "" + body.length());
            httpConnection.getOutputStream().write(body.getBytes("UTF-8"));
            if (httpConnection.getResponseCode() != 200)
                throw new IOException("Connection failed" + httpConnection.getResponseCode());
        }
    }
}
