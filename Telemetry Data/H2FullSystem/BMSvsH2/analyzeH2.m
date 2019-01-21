clear; clc; close all;

FILENAME = 'fullRaceOutside.txt';

data = importdata(FILENAME);

data = data(1:end, :);

c = 200;

vBMS = data(:, 1);
iBMS = data(:, 2);
eBMS = data(:, 5);
vFC = data(:, 7);
iFC = data(:, 8);
eFC = data(:, 9);
time = data(:, 10) ./ 1000;
tempFC = data(:, 13);
pressFC = data(:, 14);
flow = data(:, 15);
totalFlow = data(:, 16);
instantEff = data(:, 17);
avgEff = data(:, 18);

totalFlow = totalFlow - totalFlow(1);
eBMS = eBMS - eBMS(1);
eFC = eFC - eFC(1);
time = time - time(1);

h2Energy = totalFlow .* 1000 .* 119.93;

capEnergy = 0.5 .* c .* (vFC.^2 - vFC(1).^2);
totalFCEff = eBMS ./ h2Energy;
totalFCEffComp = (eBMS + capEnergy) ./ h2Energy;
plot(totalFCEffComp, '.'); hold on; grid on;
%plot(vFC);
plot(instantEff, '.');
plot(avgEff, '.');
ylim([.5, 0.65]);
legend('total','instant','avg')

figure(3);
plot(vBMS); hold on;
plot(vFC);

figure(4);
plot(iFC, vFC, '.');

figure();
plot((eBMS + capEnergy) ./ eFC); hold on;