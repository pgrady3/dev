halfScale = 65536 / 2;

data = importdata('data.txt');
t = 1 : length(data);

eAngle = data(:, 3);

demandAngle = data(:, 1);
encAngle = mod(data(:, 2) * 64 * 7, 65536);

dif = mod(-demandAngle + encAngle, 65536);

%plot(demandAngle, encAngle, '.');
%plot(dif)

round(mean(dif))