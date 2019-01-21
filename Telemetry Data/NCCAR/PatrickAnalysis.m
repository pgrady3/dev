%% Patrick Grady
%  Nov. 2017

clear; clc; close all;

%FILENAME = 'DATA1_rolling.TXT';
%FILENAME = 'DATA2_rolling.TXT';
FILENAME = 'OlgaGoodLongRuns.TXT';
%FILENAME = 'DATA_whole_lap_5.TXT';
%FILENAME = 'DATA_6_with_-weight.TXT';

ACCEL_WINDOW = 40;


data = importdata(FILENAME);

voltage = data(:, 1);
current = data(:, 2);
power = data(:, 3);
velo = data(:, 4);
energy = data(:, 5);
dist = data(:, 6);
elapsed = data(:, 10) ./ 1000;
accel = zeros(size(velo));

velo = smooth(velo, 21);
ke = 0.5 * (60.1 + 21.1 + 0.8) .* (velo .^2);

windowPoints = PatrickWindow(velo, power, elapsed);

for i = ACCEL_WINDOW + 1: length(velo) - ACCEL_WINDOW
   dv = velo(i + ACCEL_WINDOW) - velo(i - ACCEL_WINDOW);
   dt = elapsed(i + ACCEL_WINDOW) - elapsed(i - ACCEL_WINDOW);
   
   accel(i) = dv / dt;
   data(i, 11) = dv / dt;
end

%% Plot start/stop lines--------------------------------------------
figure(1); clf;
plot(elapsed, velo); hold on;
for i = 1:length(windowPoints) 
    start = elapsed(windowPoints(i, 1));
    stop = elapsed(windowPoints(i, 2));
    
    line([start, start], [0, 10], 'Color', 'black', 'LineWidth', 3);
    line([stop, stop], [0, 10], 'Color', 'red', 'LineWidth', 3);
end
title('Start/Stop Lines');xlabel('time (s)');ylabel('velocity (m/s)');

%% Plot decelerations------------------------------------------------
figure(2); clf;
for window = 1 : length(windowPoints)
   start = windowPoints(window, 1);
   stop = windowPoints(window, 2);
   
   plot(velo(start:stop), accel(start: stop)); hold on;
   ylim([-0.15 0]);
end

% Plot fit
coeffs = [-0.00035 0 -0.025];
veloSweep = linspace(0, 10, 1000);
decelFit = polyval(coeffs, veloSweep);
plot(veloSweep, decelFit);
title('Acceleration vs Velocity');
xlabel('Velocity (m/s)');ylabel('Acceleration(m/s^2)');
legend('Data 1','Data 2','Fit');

%% Plot decel vs dist
figure(3);clf;

for window = 1 : 2
   start = windowPoints(window, 1);
   stop = windowPoints(window, 2);
   
   if window == 1
       runData = data(start:stop, :);
       runData(:, 6) = runData(:, 6) - runData(1, 6) + 25;
   else
       runData = data(fliplr(start:stop), :);
       runData(:, 6) = -runData(:, 6) + runData(1, 6);
   end
   
   dist = runData(:, 6);
   accel = runData(:, 11);
   velo = runData(:, 4);
   
   subtractedAccel = accel - polyval(coeffs, velo);
   
   yyaxis left; ylabel('Acceleration (m/s^2)');
   plot(dist, subtractedAccel); hold on;
   %plot(dist, 0);
   yyaxis right; ylabel('Velocity (m/s)');
   plot(dist, velo);
end
