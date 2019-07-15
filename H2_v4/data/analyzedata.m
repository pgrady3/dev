% Shomik Verma
% analyzedata.m
% analyze fuel cell testing data

clear; close all;clc;
%% import data

data = importdata('load_shorting.txt');
data2 = importdata('base_IV.txt');
% power = data(:,2);
% saveIndex = (power<30 & power>15);
% data = data(saveIndex,:);
% data = data(9388:10447, :);
data2 = data2(1:3930, :);
flow = data(:,1);
power = data(:,2);
eff = data(:,3);
health = data(:,4);
voltage = data(:,5);
current = data(:,6);
baseI = data2(:,6);
baseV = data2(:,5);
baseT = data2(:,8);
leak = data(:,7);
time = data(:,8);
total = data(:,9);
temp = data(:,13);
pres = data(:,12);


h2energy = total*119.96e6;
% effInt = cumtrapz(time,power)./(h2energy - h2energy(1))*100;
effInt = cumtrapz(time,power)./(cumtrapz(time, flow*119.96))*100;
fprintf('overall eff: %.2f\n',effInt(end));
effAvg = mean(eff);
fprintf('avg eff: %.2f\n',effAvg);

% IV curve data
p1 =    -0.0335;
p2 =     0.4112;
p3 =    -1.9282;
p4 =     4.3558;
p5 =    -5.3833;
p6 =    18.3366;
fit_V = @(I) p1.*I.^5 + p2.*I.^4 + p3.*I.^3 + p4.*I.^2 + p5.*I + p6;

%% plot data

time = time - time(1);
time = time/1000;
baseT = baseT - baseT(1);
baseT = baseT/1000;

eff = smooth(eff, 20);
figure(1); clf;
plot(time, voltage,...
    time, current,...    
    time, power,...
    time, eff);
xlabel('Time (s)')
legend('FC Voltage','FC Current','FC Power','FC Efficiency','Location','NorthEast')
title('Fuel cell operation with supercaps')
axis([0 time(end-1) 0 100]);

figure(2); hold on
plot(power,eff,'.','MarkerSize',0.5);
xlabel('power (W)')
ylabel('Efficiency (%)')
% axis([ 0 60 0 100]);
% axis ([ 14.5 18 50 70]);
title('Fuel cell efficiency vs. power for various short frequencies');
% legend('10ms','15ms','20ms','Location','SouthEast')

%print -dpng /Users/shomikverma/Documents1/Duke/EV/publication_plots/p_eff_shorting_freq

figure(3); clf;
plot(voltage, power,'.');
xlabel('Voltage (V)')
ylabel('Power (W)')

figure(4);
placehold = zeros(size(current,1),1);
%scatter3(current, voltage,time, 5, time); hold on
% scatter3(baseI, baseV, baseT, 5, baseT);
plot3(baseI, baseV, baseT,'k.')
% scatter3(current, fit_V(current), placehold, 5, placehold)
view([0 90])
xlabel('Current')
ylabel('Voltage')
% axis([0 4.5 14.25 17])
c = colorbar;
ylabel(c, 'Time (s)')
title('IV curve for direct connection')
grad = gradient(current);
short_help = sum(grad(grad>0.05))/sum(grad>0.05);
% fprintf('short help: %f\n',short_help);

%print -dpng /Users/shomikverma/Documents1/Duke/EV/publication_plots/fast_discharge

figure(5); clf;
plot(time, temp)
xlabel('time')
ylabel('temp')

figure(6); clf;
plot(time, pres);
xlabel('time');
ylabel('Pressure')

figure(7); clf;
plot(time, effInt);
hold on;
plot(time, eff);
xlabel('time')
ylabel('eff')

% figure(8); clf;
% plot(power, eff);
