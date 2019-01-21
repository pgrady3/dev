clear; clc; close all;

%FILENAME = 'DATA1_rolling.TXT';
%FILENAME = 'DATA2_rolling.TXT';
%FILENAME = 'Run3PristineOutAndBack-Run4OutAndBackSmoothOnly.TXT';
FILENAME = 'DATA_whole_lap_5.TXT';
%FILENAME = 'DATA_6_with_-weight.TXT';

ACCEL_WINDOW = 40;


data = importdata(FILENAME);
data = data(1850:end, :);


voltage = data(:, 1);
current = data(:, 2);
power = data(:, 3);
velo = data(:, 4);
energy = data(:, 5);
dist = data(:, 6);
elapsed = data(:, 10) ./ 1000;
accel = zeros(size(velo));

energy = energy - energy(1);
dist = dist - dist(1);

velo = smooth(velo, 21);
ke = 0.5 * (60.1 + 21.1 + 0.8) .* (velo .^2);
miles = dist ./ 1609;
totalEnergy = energy - ke;

kwh = totalEnergy ./ (1000 * 3600);

mpkwh = miles ./ kwh;

plot(mpkwh);
ylim([0 500]);