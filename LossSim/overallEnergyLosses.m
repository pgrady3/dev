close all; clc;
clearvars -except electricScoreEnglish

oldScore = 1;
if exist('electricScoreEnglish', 'var')
    oldScore = electricScoreEnglish;
end

%all loss terms are in watts

v = 6.706; %nominal race speed in m/s
massCar = 21; %mass of car in kg
massDriver = 50; %mass of driver in kg
g = 9.807; %acceleration of gravity
kc = 5.38e-3; %constant wheel drag term
%kq = 1.64e-5; %quadratic wheel drag term
%kc = 1e-3; %constant wheel drag term
kq = 0.9e-5; %quadratic wheel drag term
d_wheel = 0.475; %diameter of the wheel in m

massTotal = massCar + massDriver;

totalLosses = [];
totalLossesLabels = {};

%air drag------------------------------------------------
airCDA = 0.033; %coefficient of drag area
airDensity = 1.184; %density of air at 25C and standard pressure, kg/m^3

airForce = 0.5 * airDensity * airCDA * v^2;
airPower = airForce * v;

totalLosses = [totalLosses, airPower];
totalLossesLabels{end+1} = 'External air drag';

% airRMSMultiplier = 1.0149; %rms of the velocity profile, world record attempt
% %airRMSMultiplier = 1.044; %sonoma, our first run
% airRMSLoss = airPower * (airRMSMultiplier ^ 2 - 1);
% totalLosses = [totalLosses, airRMSLoss];
% totalLossesLabels{end+1} = 'Drag RMS loss';

%rolling resistance--------------------------------------
rrCoeff = 0.0015 * 100/100; %coefficient of rolling resistance
rrForce = massTotal * g * rrCoeff; %drag force of rolling resistance in newtons
rrPower = rrForce * v; %power loss of rolling resistance in watts

totalLosses = [totalLosses, rrPower];
totalLossesLabels{end+1} = 'Tire rolling resistance';

%cornering losses----------------------------------------
ca = 120; %tire cornering stiffness, newtons per degree
cornerRadius = [30 30 30 30]; %turn radius in meters. assuming each corner turns 90 degrees
cornerVelocity = [v v v v]; %speed taken through each corner
trackLength = 1947.1; %track length in meters. Galot raceway in Benson, NC

alpha = (massTotal .* cornerVelocity.^2 ./ cornerRadius) ./ ca; %tire slip angle, degrees
corneringDragForce = ca .* alpha .^ 2 .* pi ./ 180; %traction force needed

corneringPower = corneringDragForce .* v;
corneringAveragePower = sum(corneringPower .* 0.5 .* pi .* cornerRadius ./ trackLength);

totalLosses = [totalLosses, corneringAveragePower];
totalLossesLabels{end+1} = 'Tire cornering losses';

%wheel air drag------------------------------------------
wheelOmega = v / (d_wheel / 2);
wheelAirLoss = 3 * kq * wheelOmega .^ 3; %three wheels in the car

totalLosses = [totalLosses, wheelAirLoss];
totalLossesLabels{end+1} = 'Internal wheel air drag';
%bearing drag--------------------------------------------
bearingLoss = 3 * kc * wheelOmega; %three wheels in the car

totalLosses = [totalLosses, bearingLoss];
totalLossesLabels{end+1} = 'Wheel bearing drag';
%freewheel drag------------------------------------------

% %motor losses--------------------------------------------
% motorGearRatio = 120/14;
% motorRPM = wheelOmega * motorGearRatio * 60/(2*pi);
% motorCurrent = 5;
% motorVoltage = 16;
% motorWindingResistance = 0.186;
% motorControllerResistance = 0.016;
% motorNoLoadPower_0 = 5.76;
% motorNoLoadRPM_0 = 4200;
% motorDutyCycle = sum(totalLosses) / (motorCurrent * motorVoltage);
% 
% motorResistanceLoss = (motorWindingResistance + motorControllerResistance) * motorCurrent.^2 * motorDutyCycle;
% motorNoLoadLoss = motorNoLoadPower_0 * motorRPM / motorNoLoadRPM_0 * motorDutyCycle;
% 
% totalLosses = [totalLosses, motorResistanceLoss + motorNoLoadLoss];
% totalLossesLabels{end+1} = 'Motor losses';
% 
% motorEff = 1 - (motorResistanceLoss + motorNoLoadLoss) / sum(totalLosses);
% %chain losses--------------------------------------------
% 
% chainPower_0 = 3.0;
% chainRPM_0 = 2600;
% 
% chainLoss = chainPower_0 * motorRPM / chainRPM_0 * motorDutyCycle;
% totalLosses = [totalLosses, chainLoss];
% totalLossesLabels{end+1} = 'Chain losses';

lossPoly_aeroAndBearing = [-1.06527e-08	-6.50352e-07	-1.02305e-04	1.12781e-03];
lossPoly_eddy = [-1.11897e-08	-5.30369e-06	-6.07395e-03	3.03073e-02] - lossPoly_aeroAndBearing;
lossPoly_aeroAndBearing(end) = 0;
lossPoly_eddy(end) = 0;
PlossMag_W = @(v) -polyval(lossPoly_eddy, v / (.475/2) * 60/(2*pi));
PlossMech_W = @(v) -polyval(lossPoly_aeroAndBearing, v / (.475/2) * 60/(2*pi));
motorMagneticPower = PlossMag_W(v);

totalLosses = [totalLosses, motorMagneticPower];
totalLossesLabels{end+1} = 'Motor magnetic';
%totalLosses = [totalLosses, PlossMech_W(v)];
%totalLossesLabels{end+1} = 'Mitsuba mechanical';

motorCurrent = 2.47;
%motorVoltage = 12;
motorWindingResistance = 0.14;
%motorDutyCycle = sum(totalLosses) / (motorCurrent * motorVoltage);
motorResistivePower = motorCurrent^2 * motorWindingResistance;
totalLosses = [totalLosses, motorResistivePower];
totalLossesLabels{end+1} = 'Motor resistive';


electricalPower = sum(totalLosses);
motorEff = (electricalPower - motorResistivePower - motorMagneticPower) / electricalPower;

figure;

H = pie(totalLosses);
T = H(strcmpi(get(H,'Type'),'text'));
P = cell2mat(get(T,'Position'));
set(T,{'Position'},num2cell(P*0.6,2));
P(1, 1) = -1.7;%hack to make text better
text(P(:,1),P(:,2),totalLossesLabels(:));

%fuel cell-----------------------------------------------
h2Eff = 0.584;
fuelCellLoss = electricalPower / h2Eff - electricalPower;

totalLosses = [totalLosses, fuelCellLoss];
totalLossesLabels{end+1} = 'Fuel cell losses';

%plotting and scorekeeping------------------------------
% figure;
% H = pie(totalLosses);
% T = H(strcmpi(get(H,'Type'),'text'));
% P = cell2mat(get(T,'Position'));
% set(T,{'Position'},num2cell(P*0.6,2));
% text(P(:,1),P(:,2),totalLossesLabels(:));


totalPower = sum(totalLosses);
electricScoreMetric = v ./ (electricalPower ./ 3600); %electric score in km per kWh
electricScoreEnglish = electricScoreMetric ./ 1.609; %electric score in miles per kWh
joulesPerLiterGas = 42.9e6 * 0.7646; %using constants from 2018 Eco-Marathon rules
scoreMetric = joulesPerLiterGas .* v ./ (totalPower .* 1000); %hydrogen score in km per liter of gas
scoreEnglish = scoreMetric ./ 1.609 .* 3.78541; % miles per gallon

%note, actual world record hydrogen score was 14,573 MPG, 6196 km/L

fprintf('Predicted electric score: %.1f mi/kWh, Hydrogen score: %.1f km/L\n', electricScoreEnglish, scoreMetric);
fprintf('Relative difference to last run %.3f \n', electricScoreEnglish / oldScore);