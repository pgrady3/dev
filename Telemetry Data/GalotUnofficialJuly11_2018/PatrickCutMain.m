clear; clc; close all;

files = dir('*.mat');

ACCEL_WINDOW = 40;

for file = files'
    filedata = load(file.name);
    vars = fields(filedata);
    data = filedata.(vars{1});

    voltage = data(:, 1);
    current = data(:, 2);
    power = data(:, 3);
    velo = data(:, 4);
    energy = data(:, 5);
    dist = data(:, 6);
    elapsed = data(:, 10) ./ 1000;
    accel = zeros(size(velo));

    velo = smooth(velo, 21);
    
    for i = ACCEL_WINDOW + 1: length(velo) - ACCEL_WINDOW
       dv = velo(i + ACCEL_WINDOW) - velo(i - ACCEL_WINDOW);
       dt = elapsed(i + ACCEL_WINDOW) - elapsed(i - ACCEL_WINDOW);

       accel(i) = dv / dt;
    end
    
    if contains(file.name, 'down')
        accel = accel - 0.023;
        %continue
    else
        accel = accel + 0.023;
    end
    
    figure(1);
    plot(velo, accel, 'DisplayName', strrep(file.name,'_','\_')); hold on;
    ylim([-0.1 0]);
    
    figure(2);
    plot(velo, 'DisplayName', strrep(file.name,'_','\_')); hold on;
end

figure(1);
legend(gca, 'show');
figure(2);
legend(gca, 'show');