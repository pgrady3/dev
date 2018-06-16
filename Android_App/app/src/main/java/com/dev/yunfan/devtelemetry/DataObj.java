package com.dev.yunfan.devtelemetry;

import android.util.Log;

import com.google.gson.*;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by yunfan on 4/19/17.
 */

class DataObj {
    private static final String TAG = "DataObj";

    String rawInput = "";
    String sessionName = "";
    String timeStamp = "";
    String longitude = "";
    String latitude = "";
    String heading = "";
    int msSinceStart = 0;
    double voltage = 0.0;
    double current = 0.0;
    double power = 0.0;
    double speed = 0.0;
    double mileage = 0.0;
    double energyUsed = 0.0;
    double temperature = 0.0;
    double batteryVoltage = 0.0;
    double altitude = 0.0;


    // True is OK. Let's pray we won't have false.
    boolean batteryStatus = true;

    DataObj(String input, String sessionName){
        try {
            this.sessionName = sessionName;
            SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
            timeStamp = dateFormat.format(new Date());
            String[] strings = input.split("\\s+");
            voltage = Double.parseDouble(strings[0]);
            current = Double.parseDouble(strings[1]);
            power = Double.parseDouble(strings[2]);
            speed = Double.parseDouble(strings[3]);
            energyUsed = Double.parseDouble(strings[4]);
            mileage = Double.parseDouble(strings[5]);
            temperature = Double.parseDouble(strings[6]);
            int battery = Integer.parseInt(strings[7]);
            batteryStatus = (battery == 1);
            batteryVoltage = Double.parseDouble(strings[8]);
            msSinceStart = Integer.parseInt(strings[9]);
            altitude = Double.parseDouble(strings[12]);

            rawInput = input;
        } catch (Exception e) {
            Log.e(TAG, "Conversion Error", e);
        }
    }

    public String toString(){
        Gson gson = new Gson();
        return gson.toJson(this);
    }
}
