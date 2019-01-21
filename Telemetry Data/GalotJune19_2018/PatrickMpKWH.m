clear; clc; close all;

%FILENAME = 'DATA1_rolling.TXT';
%FILENAME = 'DATA2_rolling.TXT';
%FILENAME = 'Run3PristineOutAndBack-Run4OutAndBackSmoothOnly.TXT';
FILENAME = 'run5.txt';
%FILENAME = 'DATA_6_with_-weight.TXT';

ACCEL_WINDOW = 100;


data = importdata(FILENAME);
%data = data(12180:end, :);%run 4
data = data(3490:end, :);%run 5
%data = data(6643:end, :);%run 5 begin lap2
%data = data(6475:end, :);%run 6
%data = data(4945:end, :);


voltage = data(:, 1);
current = data(:, 2);
power = data(:, 3);
velo = data(:, 4) ./ 8 .* 7;
energy = data(:, 5);
dist = data(:, 6);
elapsed = data(:, 10) ./ 1000;
lat = data(:, 11);
lon = data(:, 12);
accel = zeros(size(velo));

energy = energy - energy(1) - 2700;
dist = dist - dist(1);
elapsed = elapsed - elapsed(1);

velo = smooth(velo, 21);
ke = 0.5 * (21 + 40) .* (velo .^2);
miles = dist ./ 1609;
totalEnergy = energy - ke;

kwh = energy ./ (1000 * 3600);
kwhTE = totalEnergy ./ (1000 * 3600);

mpkwh = miles ./ kwh;
mpkwhTE = miles ./ kwhTE;

figure(1);
plot(velo);
ylim([0 10])

figure(2);
plot(mpkwh); hold on;
plot(mpkwhTE);
legend('no TE', 'TEC');
ylim([400 750]);

figure(3); clf;
plot(dist ./ elapsed); hold on;
plot(velo);
ylim([0 10]);

velo2 = zeros(length(velo), 1);
for i = 50:length(velo2)
    
   velo2(i) = (dist(i) - dist(i - 49)) ./  (elapsed(i) - elapsed(i - 49));
    
end

plot(velo ./ velo2);

figure(4);
scatter3(lat, lon, 1:length(lat));
xlim([35.315, 35.33])
ylim([-78.513 -78.508])

figure(5);
subplot(2, 1, 1);
plot(energy);
subplot(2, 1, 2);
plot(power);