%% Patrick Grady
%  Nov. 2017

clear; clc; close all;

%FILENAME = 'DATA1_rolling.TXT';
%FILENAME = 'DATA2_rolling.TXT';
%FILENAME = 'Run3PristineOutAndBack-Run4OutAndBackSmoothOnly.TXT';
%FILENAME = 'DATA_whole_lap_5.TXT';
%FILENAME = 'DATA_6_with_-weight.TXT';
FILENAME = '../Creepin-at-a-Middle-School/01.TXT';
FILENAME2= '../Creepin-at-a-Middle-School/02.TXT';
FILENAME3= '../Creepin-at-a-Middle-School/03.TXT';
ACCEL_WINDOW = 200;


data = importdata(FILENAME);
data = data(18000:20500 , :);
data2 = importdata(FILENAME2);
data2 = data2(1:5000,:);
data2(:,10) = data2(:,10) + 1500*1000;
  
data3 = importdata(FILENAME3);
data3 = data3(1:5000,:);
data3(:,10) = data3(:,10) + 2000*1000;
 
data = [data; data2; data3];

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
for i = 1:size(windowPoints) 
    start = elapsed(windowPoints(i, 1));
    stop = elapsed(windowPoints(i, 2));
    
    line([start, start], [0, 10], 'Color', 'black', 'LineWidth', 3);
    line([stop, stop], [0, 10], 'Color', 'red', 'LineWidth', 3);
end
title('Start/Stop Lines');xlabel('time (s)');ylabel('velocity (m/s)');
ylim([-0.15 12]);
%% Plot decelerations------------------------------------------------
figure(2); clf;
veloAll=0;
accelAll=0;
windowTrim = 200;
for window = 1 : size(windowPoints)
   start = windowPoints(window, 1) + windowTrim;
   stop = windowPoints(window, 2) - windowTrim;
   
   plot(velo(start:stop), accel(start: stop)); hold on;
   ylim([-0.15 0]);
   
   veloAll = [veloAll;velo(start:stop)];
   accelAll = [accelAll;accel(start: stop)];
end

% Plot fit
%coeffs = [-0.00035 0 -0.025];
veloSweep = linspace(0, 10, 1000);
%decelFit = polyval(coeffs, veloSweep);
coeffs = polyfit(veloAll,accelAll,2);

f = @(a, x) a(1) + a(2).*x.^2;
fSSR = @(a, xm, ym) sum((ym - f(a,xm)).^2);
coeffs = fminsearch(@(a) fSSR(a,veloAll,accelAll), [0,0])


St = sum(( accelAll - mean(accelAll) ).^2);
Sr = sum(( accelAll - f(coeffs,veloAll) ).^2);
r2 = (St - Sr) / St

decelFit = f(coeffs,veloSweep);%polyval(coeffs,veloSweep);
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
