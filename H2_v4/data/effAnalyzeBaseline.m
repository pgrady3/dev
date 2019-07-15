% Patrick Grady
% analyze fuel cell testing data

clear; close all; clc;
%% import data

leakRate = 0.010; %mg per sec

energyDensityH2 = 119.93e3; %joules per g

%data = importdata('base_IV.txt');
data = importdata('baseline_Sept9-2018.txt');
%data = data(1:4400, :);

vBMS = data(:, 1);
iBMS = data(:, 2);
velo = data(:,4);
eBMS = data(:, 5);
vFC = data(:, 7);
iFC = data(:, 8);
eFC = data(:, 9);
time = data(:, 10) ./ 1000;
time = time - time(1);
tempFC = data(:, 13);
pressFC = data(:, 14);
flow = data(:, 15);
instantH2 = smooth(flow*119.93,50);
instantn = iBMS.*vBMS./instantH2;
totalFlow = data(:, 16);
instantEff = data(:, 17);

power = iFC .* vFC;

flow = smooth(flow, 21);
power = smooth(power, 21);

h2energy = totalFlow* energyDensityH2;

h2power = flow .* energyDensityH2 / 1000;
leakPower = leakRate * energyDensityH2 / 1000;

voltageEff = vFC ./ 20 ./ 1.229;
totalEff = power ./ h2power;

flowByCurrent = iFC / 1.60217e-19 / 2 / 6.02214e23 * 2.01588 * 1e3 * 20; 
powerByCurrent = flowByCurrent * energyDensityH2 / 1000;


%plot(current, voltageEff); hold on;
%plot(power, power ./ (powerByCurrent + leakPower)); hold on;
%plot(power, power ./ (powerByCurrent));
plot(power, totalEff, '.');
legend('Actual efficiency', 'Efficiency before leakage')
ylim([0.4 0.7]);
grid on;
xlabel("Output power in watts")
ylabel("Efficiency");
title("Horizon 100W Modelled Efficiency - Not Including Auxiliaries")

%figure();
%plot(flowByCurrent + leakRate); hold on;
%plot(flow);