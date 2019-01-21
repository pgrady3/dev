clear; clc; close all;

TICKS_REV = 8;
ACCEL_WINDOW = 32;

filesStruct = dir('*.txt');

targetVelo = 6.7;
wheelRad = 0.475 / 2;

radSec = targetVelo / wheelRad;

for i = 1:numel(filesStruct)
    filename = filesStruct(i).name;
    
    data = importdata(filename);
    
    meterspersec = data(:, 4);
    
    time = data(:, 10) ./ 1000;
    
    velo = meterspersec / (0.475 / 2);%to rad/sec
    
    velo = smooth(velo, 21);
    
    accel = [];
    
    for i = 1:length(velo) - ACCEL_WINDOW
        i2 = i + ACCEL_WINDOW;
        accel(i) = (velo(i2) - velo(i)) / (time(i2) - time(i)); 
    end
    
    accel = smooth(accel, 21);
    
    velo = velo(1 : length(accel));
    
    I = 0.0533;%calculated based on hoop masses
    
    torque = accel * I;
    power = torque .* velo;
    
    figure(1);
    plot(velo, accel, '.', 'DisplayName', filename); hold on;

    figure(2);
    plot(velo, power, '.', 'DisplayName', filename); hold on;
end

figure(1);
xlabel('rad/sec');
ylabel('rad/sec^2');
legend(gca,'show');
ylim([-2 0]);
xlim([0 40]);
grid on;

figure(2);
xlabel('rad/sec');
ylabel('watts')
legend(gca,'show');
ylim([-3 0]);
xlim([0 40]);
grid on;