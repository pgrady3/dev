% Shomik Verma
% analyzedata.m
% analyze fuel cell testing data

clear; clf;
%% import data

data = importdata('v_eff_test2.txt');
flow = data(:,1);
power = data(:,2);
eff = data(:,3);
health = data(:,4);
voltage = data(:,5);
current = data(:,6);
leak = data(:,7);
time = data(:,8);


h2energy = flow/1000*119.96e3;
effInt = trapz(time,power)/trapz(time,h2energy)*100;
fprintf('overall eff: %.2f\n',effInt);
effAvg = mean(eff);
fprintf('avg eff: %.2f\n',effAvg);

%% plot data

time = time - time(1);
time = time/1000;

eff = smooth(eff, 20);
figure(1);
plot(time, voltage,...
    time, current,...    
    time, power,...
    time, eff);
xlabel('Time (s)')
legend('FC Voltage','FC Current','FC Power','FC Efficiency','Location','NorthEast')
title('Fuel cell operation with supercaps')
axis([0 time(end) 0 100]);

figure(2);
plot(voltage,eff,'.','MarkerSize',0.5);
xlabel('Voltage')
ylabel('Efficiency')
% axis([ 0 60 0 100]);
axis ([ 14.5 18 50 70]);
title('Efficiency vs. voltage');