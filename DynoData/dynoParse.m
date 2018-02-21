clear; clc;

data = importdata('data.txt');

voltage = data(:, 1);
current = data(:, 2);
rpm = data(:, 4);
time = data(:, 5) ./ 1000;