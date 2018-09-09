clear; close all;

FILENAME = 'run3.txt';
data = importdata(FILENAME);

% baseIVname = 'base_IV.txt';
% data_base = importdata(baseIVname);
% % data_base = data_base(1:3930, :);
% baseI = data_base(:,6);
% baseV = data_base(:,5);
% baseT = data_base(:,8);
% baseT = baseT - baseT(1);
% baseT = baseT/1000;

% IV curve data
p1 =    -0.0335;
p2 =     0.4112;
p3 =    -1.9282;
p4 =     4.3558;
p5 =    -5.3833;
p6 =    18.3366;
fit_V = @(I) p1.*I.^5 + p2.*I.^4 + p3.*I.^3 + p4.*I.^2 + p5.*I + p6;

% data = data(9228:26080, :); % middle school test
data = data(3849:28000,:);
% vFC = data(:, 7);
% data = data(gradient(vFC)<0, :);

c = 200;

vBMS = data(:, 1);
iBMS = data(:, 2);
velo = data(:,4);
eBMS = data(:, 5);
vFC = data(:, 7);
iFC = data(:, 8);
eFC = data(:, 9);
time = data(:, 10) ./ 1000;
tempFC = data(:, 13);
pressFC = data(:, 14);
flow = data(:, 15);
totalFlow = data(:, 16);
instantEff = data(:, 17);
% avgEff = data(:, 18);
avgEff = zeros(size(instantEff));

% IV curve data
p1 =    -0.0335;
p2 =     0.4112;
p3 =    -1.9282;
p4 =     4.3558;
p5 =    -5.3833;
p6 =    18.3366;
fit_V = @(I) p1.*I.^5 + p2.*I.^4 + p3.*I.^3 + p4.*I.^2 + p5.*I + p6;
minI = min(iFC);
maxI = max(iFC);
baseI = linspace(minI, maxI, 1000);
baseT = ones(size(baseI,2),1);

totalFlow = totalFlow - totalFlow(1);
eBMS = eBMS - eBMS(1);
eFC = eFC - eFC(1);
time = time - time(1);

h2Energy = totalFlow .* 1000 .* 119.93;

capEnergy = 0.5 .* c .* (vFC.^2 - vFC(1).^2);
totalFCEff = eBMS ./ h2Energy;
totalFCEffComp = (eBMS + capEnergy) ./ h2Energy;
plot(time, totalFCEffComp, '.'); hold on; grid on;
%plot(vFC);
plot(time, instantEff, '.');
plot(time, avgEff, '.');
ylim([.5, 0.65]);
legend('total','instant','avg')
title('efficiency vs. time')

figure(3); clf;
plot(time, vBMS); hold on;
plot(time, vFC);
title('voltage vs. time')

figure(4); clf;
scatter3(iFC, vFC, time,5,time); hold on;
% scatter3(baseI, baseV, baseT, 5, baseT);
scatter3(baseI, fit_V(baseI), baseT, 5, 'k');
xlim([ 0 5])
ylim([14 18])
view([0 90])
colorbar
title('IV curve')

figure(5); clf;
plot((eBMS + capEnergy) ./ eFC); hold on;

figure(6); clf;
plot(time, tempFC)
title('temp vs. time')

figure(7); clf;
plot(vFC, instantEff, '.');
title('efficiency vs. voltage')