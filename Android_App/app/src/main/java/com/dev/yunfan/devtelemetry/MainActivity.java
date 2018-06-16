package com.dev.yunfan.devtelemetry;

import android.bluetooth.BluetoothAdapter;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;


public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MAIN_UI";
    private static BluetoothAdapter btAdapter;
    private TextView speed;
    private TextView distance;
    private TextView power;
    private TextView voltage;
    private TextView timeView;
    private TextView current;
    private TextView energy;
    private TextView altitude;
    private Button button;
    private long lastPressTime = 0;
    private double lastPressEnergy = 0.0;
    private double lastPressDistance = 0.0;

    private BtDataService dataService;
    private dataConnection connection;
    private UiThread uiThread;
    boolean isBound = false;

    private class UiThread extends Thread{
        @Override
        public void run() {
            try {
                while (!isInterrupted()) {
                    Thread.sleep(300);
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            updateUi();
                        }
                    });
                }
            } catch (InterruptedException e) {
                Log.e(TAG, "Cannot update UI", e);
            }
        }
    }

    private class dataConnection implements ServiceConnection {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            BtDataService.LocalBinder binder = (BtDataService.LocalBinder) service;
            dataService = binder.getService();
            isBound = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            isBound = false;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        speed = (TextView) findViewById(R.id.speed);
        distance = (TextView) findViewById(R.id.distance);
        power = (TextView) findViewById(R.id.power);
        voltage = (TextView) findViewById(R.id.voltage);
        current = (TextView) findViewById(R.id.current);
        altitude = (TextView) findViewById(R.id.altitude);
        timeView = (TextView) findViewById(R.id.timeView);
        energy = (TextView) findViewById(R.id.energyUsed);

        button = (Button) findViewById(R.id.startButton);
        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                lastPressTime = System.currentTimeMillis();
                if (isBound) {
                    DataObj obj = dataService.getMostRecent();
                    lastPressDistance = obj.mileage;
                    lastPressEnergy = obj.energyUsed;
                }
            }
        });

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


        startService(new Intent(this, BtDataService.class));
        connection = new dataConnection();
    }

    @Override
    protected void onStart() {
        super.onStart();
        bindService(new Intent(this, BtDataService.class), connection, Context.BIND_AUTO_CREATE);
        uiThread = new UiThread();
        uiThread.start();
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
        uiThread.interrupt();
        if (isBound)
            unbindService(connection);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        stopService(new Intent(this, BtDataService.class));
    }

    private void updateUi() {
        Log.i(TAG, "Trying to update UI");
        String result = "";
        DataObj obj = null;
        if (isBound)
            obj = dataService.getMostRecent();
        if (obj != null) {
            speed.setText(String.valueOf(obj.speed));
            distance.setText(String.valueOf(obj.mileage - lastPressDistance));
            power.setText(String.valueOf(obj.power));
            voltage.setText(String.valueOf(obj.voltage));
            current.setText(String.valueOf(obj.current));
            altitude.setText(String.valueOf(obj.altitude));
            energy.setText(String.valueOf(obj.energyUsed - lastPressEnergy));
            if (lastPressTime == 0)
                timeView.setText(String.valueOf(obj.msSinceStart / 1000));
            else
                timeView.setText(String.valueOf((System.currentTimeMillis() - lastPressTime) / 1000));
        }
        Log.i(TAG, "Update UI successful.");
    }
}
