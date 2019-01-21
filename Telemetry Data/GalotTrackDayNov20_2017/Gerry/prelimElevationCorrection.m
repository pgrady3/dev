%%  Gerry Chen
%   Nov 23, 2017
%   Script to analyze and account for the elevation change
clear;clc;close all;

%%
smoothingParams = linspace(.01,.5,20);

%% Load Data
load ../run1_down;
data = good2;

voltage = data(:, 1);
current = data(:, 2);
power = data(:, 3);
velo = data(:, 4);
energy = data(:, 5);
dist = data(:, 6)-data(1,6);
t = data(:, 10) ./ 1000;

%% x, v, and a fits
figure(1);
for k = 1:length(smoothingParams)
    distFit = createFit(t,dist,smoothingParams(k));
    velFit = splineDeriv(distFit);
    accFit= splineDeriv(distFit,2);
    vel = velFit(t);
    acc = accFit(t);
    
    subplot(3,1,1);
	plot(t,distFit(t));
    title('x vs t'); xlabel('t (s)'); ylabel('x (m/s)');
    ylim([0,max(dist)*1.1]);
    subplot(3,1,2);
    plot(t,velo,'k.',t,velFit(t));
    title('v vs t'); xlabel('t (s)'); ylabel('v (m/s)');
    ylim([0,max(velo)*1.1]);
    subplot(3,1,3);
    plot(t,accFit(t));
    title('a vs t'); xlabel('t (s)'); ylabel('a (m/s)');
    ylim([-.1,.1])
    xvaFrames(k) = getframe(gcf);
end
clf
axes('Position',[0 0 1 1])
movie(xvaFrames,1,10);

% %% test fits
% fprintf('back test integrals:\n');
% fprintf('\tdist top-bot:\t%f\n',dist(end)-dist(1));
% fprintf('\tvel integral:\t%f\n',trapz(t,vel));
% fprintf('\tacc integral:\t%f\n',trapz(t,cumtrapz(t,acc)));
% 
% figure();
% plot(t,dist-dist(1),'k-',...
%     t,cumtrapz(t,vel),'b-',...
%     t,cumtrapz(t,cumtrapz(t,acc)),'m-');
% title('Fit Comparisons'); xlabel('t (s)'); ylabel('\Delta x');
% legend({'$$\Delta x$$','$$\int_0^t v dt$$','$$\int_0^t \int_0^t a\ dt dt$$'},...
%     'Interpreter','latex','Location','SouthEast');

%% isolate regions
accelInd = find(acc>0 & t>200);
decelInd = find(acc<0 & t>220);
decelInd = decelInd(fix(length(decelInd)*.1):fix(length(decelInd)*.9));

%% a vs v
figure();
for k = 1:length(smoothingParams)
    distFit = createFit(t,dist,smoothingParams(k));
    velFit = splineDeriv(distFit);
    accFit= splineDeriv(distFit,2);
    vel = velFit(t);
    acc = accFit(t);
    plot(vel(decelInd), acc(decelInd),'r-')
    title('Accel vs Velo');xlabel('v (m/s)');ylabel('a (m/s^2');
    ylim([-.2,.15]);
    avFrames(k) = getframe();
end
movie(avFrames,5,10)

%% save plots
save('1downPlots','xvaFrames','avFrames')