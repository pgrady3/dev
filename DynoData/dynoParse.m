clear; clc; close all;

ACCEL_WINDOW = 10;
ROT_INERTIA = 0.8489;

PARASITIC_LOSSES = [-0.000000174009895   0.000029033561439  -0.003222070361203  -0.030973207638652];%just air + bearing
%PARASITIC_LOSSES = [-0.000000132005837   0.000026535670679  -0.004129207335037  -0.063561570528803];%air + bearing + chain + hysterisis

data = importdata('20V12A.txt');

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

accel = smooth(accel, 21);

accelComp = accel - polyval(PARASITIC_LOSSES, velo);


torque = ROT_INERTIA .* accelComp;
mPower = torque .* velo;
eff = mPower ./ ePower;

figure(1);

plot(ePower);
hold on;
plot(mPower);

figure(2);
plot(rpm, eff);
ylim([0.6, 1]);