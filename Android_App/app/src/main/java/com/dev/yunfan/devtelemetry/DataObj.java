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
    String sessionName = "";
    String timeStamp = "";
    String longitude = "";
    String latitude = "";
    int msSinceStart = 0;
    double voltage = 0.0;
    double current = 0.0;
    double power = 0.0;
    double speed = 0.0;
    double mileage = 0.0;
    double energyUsed = 0.0;
    double currentDemand = 0.0;
    //double fuelCellVoltage = 0.0;
    //double fuelCellCurrent = 0.0;
    //double fuelCellEnergy = 0.0;
    //double fuelCellTemp = 0.0;
    //double h2Pressure = 0.0;
    //double h2Flow = 0.0;
    //double h2Total = 0.0;
    //double h2Efficiency = 0.0;

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
            //fuelCellVoltage = Double.parseDouble(strings[6]);
            //fuelCellCurrent = Double.parseDouble(strings[7]);
            //fuelCellEnergy = Double.parseDouble(strings[8]);
            msSinceStart = Integer.parseInt(strings[9]);
            latitude = strings[10];
            longitude = strings[11];
            currentDemand = Double.parseDouble(strings[7]);
            //fuelCellTemp = Double.parseDouble(strings[12]);
            //h2Pressure = Double.parseDouble(strings[13]);
            //h2Flow = Double.parseDouble(strings[14]);
            //h2Total = Double.parseDouble(strings[15]);
            //h2Efficiency = Double.parseDouble(strings[16]);
        } catch (Exception e) {
            Log.e(TAG, "Conversion Error", e);
        }
    }

    public String toString(){
        Gson gson = new Gson();
        return gson.toJson(this);
    }
}
