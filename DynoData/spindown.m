clear; clc; %close all;

ACCEL_WINDOW = 54;
%ROT_INERTIA = 0.8489;

%data = importdata('unloaded_spindown.txt');
data = importdata('unloaded_spindown_5_28.txt');
%data = importdata('motorUnplugged_spindown.txt');

rpm = data(:, 4);

for i = 1:length(rpm) - 2%fix glitches in rpm readout
   if (rpm(i) > 0) && (rpm(i+2) > 0) && (rpm(i+1) == 0)
       rpm(i+1) = rpm(i);
   end
end

rpm = smooth(rpm, 21);

velo = rpm * 2 * pi / 60;
time = data(:, 6) ./ 1000;

accel = zeros(size(velo));

for i = 1:length(velo) - ACCEL_WINDOW
    i2 = i + ACCEL_WINDOW;
    accel(i) = (velo(i2) - velo(i)) / (time(i2) - time(i)); 
end

%plot(velo);

startWindow = 0;
endWindow = 0;

for i = 50:length(velo)
   if velo(i - 49) < velo(i) && velo(i) > velo(i + 49)
       startWindow = i + 200;
       break;
   end
end

for i = startWindow:length(velo)
   if velo(i - 49) < 4 && velo(i) < 4
       endWindow = i;
       break;
   end
end

figure; clf;
plot(velo); hold on;
line([startWindow, startWindow], [0, 100], 'Color', 'black', 'LineWidth', 3);
line([endWindow, endWindow], [0, 100], 'Color', 'red', 'LineWidth', 3);

veloCut = velo(startWindow:endWindow);
accelCut = accel(startWindow:endWindow);
rpmCut = rpm(startWindow:endWindow);

figure;
plot(rpmCut, accelCut);

coeffs = polyfit(veloCut, accelCut, 3)
hold on;
plot(rpmCut, polyval(coeffs, veloCut));