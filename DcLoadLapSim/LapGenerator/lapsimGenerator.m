clear; clc; close all;

FILENAME = 'officialRuns.TXT';
data = importdata(FILENAME);


%data = data(93710+373:120286, :); %official run3
%data = data(70270:86090, :); %official run2
data = data(25845:52230, :); %official run1, flowmeter clear to end

%data = data(16810:24730+16810, :); %unofficial1, flowmeter clear to end
%data = data(16710:22520+16710, :); %unofficial2

vBMS = data(:, 1);
iBMS = data(:, 2);
energy = data(:, 5);
vFC = data(:, 7);
iFC = data(:, 8);
eFC = data(:, 9);
tempFC = data(:, 13);
pressFC = data(:, 14);
flow = data(:, 15);
totalFlow = data(:, 16);
instantEff = data(:, 17);

power = data(:, 3);
velo = data(:, 4);
dist = data(:, 6);
elapsed = data(:, 10) ./ 1000;
lat = data(:, 11);
lon = data(:, 12);


totalFlow = totalFlow - totalFlow(1);
eFC = eFC - eFC(1);
energy = energy - energy(1);
dist = dist - dist(1);
elapsed = elapsed - elapsed(1);
h2Energy = totalFlow .* 1000 .* 119.93;
instantEff = movmean(instantEff, 21);

plot(elapsed, power);

startStopArr = [];

startPos = -1;

for i = 1:length(power)
    if startPos < 0 && power(i) > 40
        startPos = i;
    end
    
    if startPos >= 0 && power(i) < 10
        avgPower = mean(power(startPos:i));
        startTime = elapsed(startPos);
        stopTime = elapsed(i);
        startStopArr = [startStopArr; startTime, stopTime, avgPower];
        startPos = -1;
    end
end

csvwrite("lapData.txt", startStopArr);
startStopArr