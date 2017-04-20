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
import android.widget.TextView;



public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MAIN_UI";
    private static BluetoothAdapter btAdapter;
    private TextView mainText;
    private BtDataService dataService;
    private dataConnection connection;
    boolean isBound = false;

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
        startService(new Intent(this, BtDataService.class));
    }

    @Override
    protected void onStart() {
        super.onStart();
        bindService(new Intent(this, BtDataService.class), connection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.e(TAG, dataService.getMostRecent().toString());
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
    }
}
