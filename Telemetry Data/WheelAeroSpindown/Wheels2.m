clear; clc; close all;

TICKS_REV = 8;
ACCEL_WINDOW = 4;

filesStruct = dir('*.txt');

targetVelo = 6.7;
wheelRad = 0.475 / 2;

radSec = targetVelo / wheelRad;

for i = 1:numel(filesStruct)
    filename = filesStruct(i).name;
    
    data = importdata(filename);
    
    time = data ./ 1000;
    ticks = 1:length(time);
    radians = ticks  ./ TICKS_REV .* 2 * pi;
    
    velo = zeros(size(ticks));
    accel = zeros(size(ticks));
    
    for i = 1:length(ticks) - TICKS_REV
        i2 = i + TICKS_REV;
        velo(i) = (radians(i2) - radians(i)) / (time(i2) - time(i)); 
    end
    
    velo = smooth(velo, 21);
    
    for i = 1:length(ticks) - ACCEL_WINDOW
        i2 = i + ACCEL_WINDOW;
        accel(i) = (velo(i2) - velo(i)) / (time(i2) - time(i)); 
    end
    
    accel = smooth(accel, 21);
    
    I = 0.021;
    
    torque = accel * I;
    power = torque .* velo;
    
    linearVelo = velo * 0.475/2;
    
    figure(1);
    plot(linearVelo, accel, '.', 'DisplayName', filename); hold on;

    figure(2);
    plot(linearVelo, power, '.', 'DisplayName', filename); hold on;
end

figure(1);
xlabel('rad/sec');
ylabel('rad/sec^2');
legend(gca,'show');
ylim([-2 0]);
xlim([0 10]);

figure(2);
xlabel('rad/sec');
ylabel('watts')
legend(gca,'show');
ylim([-4 0]);
xlim([0 10]);