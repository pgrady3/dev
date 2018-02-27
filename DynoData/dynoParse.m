clear; clc;

data = importdata('data.txt');

voltage = data(:, 1);
current = data(:, 2);
rpm = data(:, 4);
throttle = data(:, 5);
time = data(:, 6) ./ 1000;