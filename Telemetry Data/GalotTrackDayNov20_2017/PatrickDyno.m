clear; clc; close all;

%FILENAME = 'DATA1_rolling.TXT';
%FILENAME = 'DATA2_rolling.TXT';
%FILENAME = 'Run3PristineOutAndBack-Run4OutAndBackSmoothOnly.TXT';
FILENAME = 'DATA_whole_lap_5.TXT';
%FILENAME = 'DATA_6_with_-weight.TXT';

EFF_WINDOW = 10;


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

ke = 0.5 .* (60.1 + 21.1 + 0.8) .* (velo .^ 2);
eff = zeros(size(ke));


for i = EFF_WINDOW + 1 : length(ke)
   dElec = energy(i) - energy(i - EFF_WINDOW); 
   dMech = ke(i) - ke(i - EFF_WINDOW);
   
   if(min(power(i-EFF_WINDOW:i)) < 50)
       continue
   end
   
   eff(i) = dMech / dElec;
end

clf
subplot(2, 1, 1);
plot(eff);
ylim([0 1]);

subplot(2, 1, 2);
plot(power);