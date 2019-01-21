%%  Gerry Chen
%   Nov 23, 2017
%   Analyzes the deceleration portions of data taken on Nov 19
%   Purpose: obtain rolling coefficient (mu) and drag coefficient (Cd)

%% setup
clear; clc; close all;

files = dir('datas/run*.mat');

lineStyles = {'--','-'};

allVels = {[],[]};
allAccs = {[],[]};

%% Data Loading and Plotting
for file = files'
    % load data
    filedata = load([file.folder,'/',file.name]);
    vars = fields(filedata);
    data = filedata.(vars{1});
    isDown = contains(file.name,'down');
    
    power = data(:, 3);
    dist = data(:, 6);
    t = data(:, 10) ./ 1000;
    t = t - t(1);
    
    distFit = createFit(t,dist,.01);
    velFit = splineDeriv(distFit);
    accFit = splineDeriv(distFit,2);
    vel = velFit(t);
    acc = accFit(t);
    
    % isolate free-run times
    decelInd = find(power<1 & vel>1);
    decelInd = decelInd(fix(length(decelInd)*.25):fix(length(decelInd)*.75));
    
    % Plot Data
    figure(1);
    plot(vel(decelInd), acc(decelInd),...
        'LineStyle',lineStyles{isDown+1},...
        'DisplayName', strrep(file.name,'_','\_')); hold on;
    
    figure(2);
    plot(t(decelInd),vel(decelInd),...
        'LineStyle',lineStyles{isDown+1},...
        'DisplayName', strrep(file.name,'_','\_')); hold on;
    allVels{isDown+1} = [allVels{isDown+1};vel(decelInd)];
    allAccs{isDown+1} = [allAccs{isDown+1};acc(decelInd)];
end

% legends
figure(1);
title('a vs v');xlabel('v (m/s)');ylabel('a (m/s^2)');
legend(gca, 'show','Location','NorthWest');
figure(2);
title('v vs t');xlabel('t (s)');ylabel('v (m/s)');
legend(gca, 'show');

%% regression
% a is air drag
% b is dummy - shouldn't really be there
% c is rolling resistance
% d is wind speed
% e is slope
form = @(a,b,c,d,e,v) a.*(v-d).^2 + b.*(v-d) + c + e + ...
    (c>0)*1e3 + abs(b-0)*1e3; % constraints
prediction = @(p,vel,down) form(p(1),p(2),p(3),...
    p(4)*(down*2-1),(down-1/2)*p(5),vel);
SSEfunc = @(p) sum((prediction(p,allVels{1},0)-allAccs{1}).^2)+...
    sum((prediction(p,allVels{2},1)-allAccs{2}).^2);
guess = [-.0006,0,-.003,0,0.02];
[coefs,SSEfound] = fminsearch(@(p) SSEfunc(p),guess);
SSD = sum((allAccs{1}-mean([allAccs{1};allAccs{2}])).^2)+...
    sum((allAccs{2}-mean([allAccs{1};allAccs{2}])).^2);
fprintf('a = (%e)v^2 + (%e)v + (%e)\n',coefs(1:3));
fprintf('a due to elevation each direction: %f m/s^2\n',coefs(5)/2);
fprintf('wind speed: %f m/s^2\n',coefs(4));
fprintf('R^2 value: %f\n',1-SSEfound/SSD);

figure(1);
plot(allVels{1},prediction(coefs,allVels{1},0),'b.')
plot(allVels{2},prediction(coefs,allVels{2},1),'r.')

figure(3);clf
plot(allVels{2}-coefs(4),allAccs{2}-coefs(5)/2,'LineStyle','none',...
    'Marker','.','MarkerSize',1);hold on;
plot(allVels{1}+coefs(4),allAccs{1}+coefs(5)/2,'LineStyle','none',...
    'Marker','.','MarkerSize',1,'MarkerFaceColor','red')
velVals = linspace(0,10);
plot(velVals,prediction(coefs,velVals,.5),'k-')
title('a vs v (elevation corrected)');
ylabel('a - corrected');xlabel('v');

%% conclusions
mass = 60.1 + 21.1 + 0.8;
g = 9.81;
crossArea = .3;
rho = 1.225;

dragAccCoef = coefs(1);
dragForce = dragAccCoef*mass;
rollingRes = coefs(3);
elevationAcc = coefs(5)/2;
dragCoef = -dragAccCoef*mass/rho/crossArea*2;
frictionForce = rollingRes*mass;
mu = -rollingRes/g;
trackSlope = asind(elevationAcc/g);

fprintf('\n\n');
fprintf('Drag Force (N)          = (%e)*v^2\n',dragForce);
fprintf('Drag Coefficient        = %f\n',dragCoef);
fprintf('Rolling Resistance      = %f N\n',frictionForce);
fprintf('Friction Coefficient mu = %f\n',mu);
fprintf('Slope of Track (deg)    = %f\n',trackSlope);

save('values',...
    'mu','dragCoef','dragForce','mass',...
    'g','crossArea','rho','frictionForce');