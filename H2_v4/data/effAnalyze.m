% Patrick Grady
% analyze fuel cell testing data

clear; close all; clc;
%% import data

leakRate = 0.010; %mg per sec

energyDensityH2 = 119.93e3; %joules per g

data = importdata('base_IV.txt');
data = data(1:4400, :);

flow = data(:,1);%milligrams per second
power = data(:,2);
eff = data(:,3);
health = data(:,4);
voltage = data(:,5);
current = data(:,6);
time = data(:,8);
leak = data(:,7);
total = data(:,9);
temp = data(:,13);
pres = data(:,12);


h2energy = total* energyDensityH2;

h2power = flow .* energyDensityH2 / 1000;
leakPower = leakRate * energyDensityH2 / 1000;

voltageEff = voltage ./ 20 ./ 1.229;
totalEff = power ./ h2power;

flowByCurrent = current / 1.60217e-19 / 2 / 6.02214e23 * 2.01588 * 1e3 * 20; 
powerByCurrent = flowByCurrent * energyDensityH2 / 1000;


%plot(current, voltageEff); hold on;
plot(power, power ./ (powerByCurrent + leakPower)); hold on;
plot(power, power ./ (powerByCurrent));
legend('Actual efficiency', 'Efficiency before leakage')
ylim([0.4 0.7]);
grid on;
xlabel("Output power in watts")
ylabel("Efficiency");
title("Horizon 100W Modelled Efficiency - Not Including Auxiliaries")

%figure();
%plot(flowByCurrent + leakRate); hold on;
%plot(flow);