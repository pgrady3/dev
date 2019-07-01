clear;clc;close all;

motorEff = 0.95;
massTotal = 21 + 50;
v0 = 7;
cornerRadius = 30;
ca = 120; %tire cornering stiffness, newtons per degree
vtarget = linspace(2, v0, 100);

alpha = @(v) (massTotal .* v.^2 ./ cornerRadius) ./ ca; %tire slip angle, degrees
corneringDragForce = @(v) ca .* alpha(v) .^ 2 .* pi ./ 180; %traction force needed

lossCorner = @(v) corneringDragForce(v) * cornerRadius * pi;

lossNoRegen = lossCorner(v0);
lossMotorRegen = @(v, v0) 0.5 .* massTotal .* (v0.^2 - v.^2) * (1-motorEff.^2);
lossTotalRegen = lossMotorRegen(vtarget, v0) + lossCorner(vtarget);

plot(vtarget, lossTotalRegen ./ lossNoRegen); hold on;
%plot(vtarget, lossMotorRegen(vtarget, v0) ./ lossTotalRegen);