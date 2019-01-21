load DATA1

%% cut irrelevant data
stopped = v<.1;
moving = ~stopped;
moving(1) = [];
startInds = find(stopped(1:end-1) & moving);
stopped(1:2) = [];
stopInds = find(stopped & moving(1:end-1));

runInds = [startInds,stopInds(2:end)]; % manually coded!!!
runs={};
for i=1:size(runInds,1)
    runs{i} = runInds(i,1):runInds(i,2);
end
run1 = runs{2};
inds = t(run1)>545 & t(run1)<555;
run1cut = run1(inds);

%% smooth
fprintf('smoothing P\n')
Psmooth = smooth(P,100);
fprintf('smoothing v\n')
vsmooth = smooth(v,100);
fprintf('smoothing odo\n')
odosmooth = smooth(odo,100);
fprintf('smoothing E\n')
Esmooth = smooth(E,100);

%% compute new variables
fprintf('computing new variables\n')
t = t/1000;
cumScore = odo./E / 1600*1000*3600; % m/J to mi/kWh
score = diff(cumScore) / 1600*1000*3600;
score = smooth(score,100,'sgolay');
a = diff(vsmooth)./diff(t);
a(end+1) = a(end);
asmooth = smooth(a,100);

%% some plots
fprintf('plotting\n')
figure(1);clf;
plot(t(run1),v(run1))
xlabel('t');
ylabel('v');

figure(2);clf;
plot(t(run1),score(run1))
xlabel('t');
ylabel('mi/kwh');

figure(3);clf;
plot(vsmooth(run1),a(run1))
xlabel('v')
ylabel('a')

figure(4);clf;
plot(t(run1),odo(run1))
xlabel('t');
ylabel('distance traveled')

figure(5);clf;
plot(t(run1),P(run1))
xlabel('t');
ylabel('P');

%% save
fprintf('saving\n')
for i = 1:size(runs)
    data(i).t = t(runs{i});
    data(i).a = asmooth(runs{i});
    data(i).aRaw = diff(v(runs{i}))./diff(t(runs{i}));
    data(i).v = vsmooth(runs{i});
    data(i).vRaw = v(runs{i});
    data(i).x = odosmooth(runs{i});
    data(i).xRaw = odo(runs{i});
    data(i).score = score(runs{i});
    data(i).V = V(runs{i});
    data(i).I = I(runs{i});
    data(i).P = Psmooth(runs{i});
    data(i).PRaw = P(runs{i});
    data(i).E = Esmooth(runs{i});
    data(i).ERaw = E(runs{i});
end
save data1_processed