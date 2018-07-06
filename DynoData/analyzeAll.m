clear; clc; close all;

ACCEL_WINDOW = 100;
ROT_INERTIA = 0.8489;

PARASITIC_LOSSES = [-0.000000174009895   0.000029033561439  -0.003222070361203  -0.030973207638652];%just air + bearing
%PARASITIC_LOSSES = [-0.000000132005837   0.000026535670679  -0.004129207335037  -0.063561570528803];%air + bearing + chain + hysterisis
PARASITIC_LOSSES = [-0.000000180290722   0.000029520905721  -0.003219018582889  -0.030440657319996];%fixed velocity

filesStruct = dir('16V/*.txt');

for i = 1:numel(filesStruct)
    filename = filesStruct(i).name;
    filePath = strcat(filesStruct(i).folder, '\', filename)
    
    data = importdata(filePath);
    
    voltage = data(:, 1);
    current = data(:, 2);
    rpm = data(:, 4);% .* 53 ./ 54;

    for i = 1:length(rpm) - 2%fix glitches in rpm readout
       if (rpm(i) > 0) && (rpm(i+2) > 0) && (rpm(i+1) == 0)
           rpm(i+1) = rpm(i);
       end
    end

    rpm = smooth(rpm, 21);
    rpmMotor = rpm .* 60 ./ 14;

    velo = rpm * 2 * pi / 60;
    throttle = data(:, 5);
    time = data(:, 6) ./ 1000;

    ePower = voltage .* current;
    ePower = smooth(ePower, 81);

    accel = zeros(size(velo));

    for i = 1:length(velo) - ACCEL_WINDOW
        i2 = i + ACCEL_WINDOW;
        accel(i) = (velo(i2) - velo(i)) / (time(i2) - time(i));
    end

    accel = smooth(accel, 41);

    accelComp = accel - polyval(PARASITIC_LOSSES, velo);

    torque = ROT_INERTIA .* accelComp;
    mPower = torque .* velo;
    eff = mPower ./ ePower;
    
    figure(1);
    plot(rpmMotor, eff, '.', 'DisplayName', filename); hold on;
    
    figure(2);
    scatter3(rpmMotor, mPower, eff, '.', 'DisplayName', filename); hold on;
    
    figure(3);
    plot(rpmMotor, current, '.', 'DisplayName', filename); hold on;
end

figure(1);
legend(gca,'show');
grid on;
ylim([0.2, 1]);

figure(2);
legend(gca,'show');
grid on;
zlim([0.84, 0.9]);
%ylim([0, 5]);
%xlim([0, 1200]);

figure(3);
legend(gca,'show');
grid on;