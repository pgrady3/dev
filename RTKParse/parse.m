clear; clc; close all;

data = importdata('NCCAR.csv');

data=data.data;


%fixed = data(:, 5) < 55.40;%better than 5cm accuracy

lockedData = data(:, :);

%lockedData = lockedData(90:600, :);

lat = lockedData(:, 2);
lon = lockedData(:, 3);
alt = lockedData(:, 4);

scatter3(lat, lon, alt);

outputCSV = [lat, lon, alt];
%dlmwrite('track.csv', outputCSV, 'delimiter', ',', 'precision', 11); 