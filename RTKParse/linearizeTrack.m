clear; clc; close all;

trackData = importdata('sonoma.csv');
trackData = trackData.data;
trackData = trackData(1500:18280, :);

%trackData = importdata('galot3.csv');
%trackData = trackData.data;
%trackData = trackData(1:2550, :);

lat = trackData(:, 2);
lon = trackData(:, 3);
elev = trackData(:, 4);

[y, x, z] = geodetic2ned(lat, lon, elev, lat(1), lon(1), elev(1), referenceEllipsoid('GRS80','m'));

x = smooth(x, 5);
y = smooth(y, 5);
z = -smooth(z, 5);

%x = downsample(x, 20);
%y = downsample(y, 20);
%z = downsample(z, 20);

scatter3(x, y, z);
xlabel('x');
ylabel('y');

rawTrack = [x, y, z];

totalDist = 0;
distLog = [];
track = rawTrack(1, :);
trackElev = [0, 0];

for i = 2:length(rawTrack)
   dist = norm(track(end, :) - rawTrack(i, :));
   
   if (totalDist + dist) / 5 > size(track, 1)
      totalDist = totalDist + dist;
      track = [track; rawTrack(i, :)];
      trackElev = [trackElev; totalDist, rawTrack(i, 3)];
   end
end

figure(2);
scatter3(track(:, 1), track(:, 2), track(:, 3));

%csvwrite('sonomaTrack.csv', track);
%csvwrite('sonomaTrackElev.csv', trackElev);

figure;
plot(trackElev(:, 1), trackElev(:, 2));
xlabel('Distance along track in m');
ylabel('Relative elevation in m');
grid on;