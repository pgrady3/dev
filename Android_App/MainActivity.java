package com.dev.yunfan.devtelemetry;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Set;
import java.util.UUID;


public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MAIN_UI";
    // This is just a random UUID created for our use.
    private static final UUID MY_UUID = UUID.randomUUID();
    private static BluetoothAdapter btAdapter;
    private static BluetoothSocket btSocket;
    private TextView mainText;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mainText = (TextView) findViewById(R.id.MAIN_TEXT);
        btAdapter = BluetoothAdapter.getDefaultAdapter();
        if (btAdapter == null) {
            Log.e(TAG, "Bluetooth not supported by this phone!");
        } else if (!btAdapter.isEnabled()) {
            Log.e(TAG, "Bluetooth is not enabled!");
            // FUTURE: The following line might changes to make it run in a service.
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            // The following line means don't start activity until the bluetooth is enabled.
            startActivityForResult(enableBtIntent, 1);
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        connect();
        ConnectedThread btComm = new ConnectedThread();
        btComm.run();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
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
            } catch (IOException refE){
                Log.e(TAG, "Reflection failed", refE);
                disconnect();
            } catch (Exception otherE){
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

    private class ConnectedThread extends Thread {
        private InputStream inStream;
        private OutputStream outStream;

        private ConnectedThread() {
            if (btSocket == null) {
                Log.e(TAG, "No socket established in ConnectedThread constructor.");
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
                    Log.e(TAG, "Received Data: " + result);
                    mainText.setText(result);
                } catch (IOException e) {
                    Log.e(TAG, "disconnected", e);
                    disconnect();
                    break;
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

}
