clear; clc; close all;

ACCEL_WINDOW = 54;
ROT_INERTIA = 0.8489;

data = importdata('unloaded_spindown.txt');

voltage = data(:, 1);
current = data(:, 2);
rpm = data(:, 4);

for i = 1:length(rpm) - 2%fix glitches in rpm readout
   if (rpm(i) > 0) && (rpm(i+2) > 0) && (rpm(i+1) == 0)
       rpm(i+1) = rpm(i);
   end
end

rpm = smooth(rpm, 21);

velo = rpm * 2 * pi / 60;
throttle = data(:, 5);
time = data(:, 6) ./ 1000;

ePower = voltage .* current;

accel = zeros(size(velo));

for i = 1:length(velo) - ACCEL_WINDOW
    i2 = i + ACCEL_WINDOW;
    accel(i) = (velo(i2) - velo(i)) / (time(i2) - time(i)); 
end

torque = ROT_INERTIA .* accel;
mPower = torque .* velo;

plot(ePower);
hold on;
plot(mPower);