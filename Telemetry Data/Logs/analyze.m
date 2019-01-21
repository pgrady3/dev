clear;clc;

FILE = 'data.txt';
SLOTS_REV = 48;
WHEEL_CIRC = .478 * pi;
VELO_WINDOW = 48;
ACCEL_WINDOW = 48 * 8;

times = importdata(FILE) / 1e6;
%times = times(1:10000);

%times = times(17000:18600);

totalDist = length(times) * WHEEL_CIRC / SLOTS_REV;
dist = linspace(0, totalDist, length(times))';

%--------------------------------------------------------------------
%Derivate position to velocity
%--------------------------------------------------------------------
velo = zeros(length(dist) - VELO_WINDOW, 1);    %velo in m/s
for i = 1 : length(velo)
    velo(i) = (dist(i + VELO_WINDOW) - dist(i)) / (times(i + VELO_WINDOW) - times(i));
end

%--------------------------------------------------------------------
%Derivate velocity to acceleration
%--------------------------------------------------------------------
accel = zeros(length(velo) - ACCEL_WINDOW, 1);
for i = 1 : length(accel)
    accel(i) = (velo(i + ACCEL_WINDOW) - velo(i)) / (times(i + ACCEL_WINDOW) - times(i));
end

outIndex = 1;
for i = 1 : length(accel)
    if accel(i) < -0.05 & accel(i) > -0.3
       accelFiltered(outIndex, 1) = accel(i);
       accelFiltered(outIndex, 2) = velo(i);
       outIndex = outIndex + 1;
    end
end

%Accel vs Velo graph
figure(1); clf;
%plot(accelFiltered(:, 2), accelFiltered(:, 1), '.');
veloCut = velo(1:length(accel));

plot(veloCut, accel, '.');

axis([2 7 -0.2 -0.1]);
xlabel('Velocity in m/s'); ylabel('Acceleration in m/s^2');

%plot(velo);