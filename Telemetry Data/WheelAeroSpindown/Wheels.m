
clear;
dat1=load('data6.txt')
dat1=dat1./1e3;
for k=1:(numel(dat1)-12)
time1(k)=dat1(k+12)-dat1(k);
end
speed1=1./time1;
for k=1:length(speed1)-8
delspeed1=speed1(k+8)-speed1(k);
end
acc1=delspeed1./(8.*time1(1:end-8));
%%
dat2=load('data13cover.txt')
dat2=dat2./1e3;
for k=1:(numel(dat2)-8)
time2(k)=dat2(k+8)-dat2(k);
end
speed2=1./time2;
for k=1:length(speed2)-8
delspeed2=speed2(k+8)-speed2(k);
end
acc2=delspeed2./(8.*time2(1:end-8));
%%
figure(1);clf
plot(dat1(1:end-12),speed1,'k.',dat2(1:end-8),speed2,'b.')
xlabel('time,s')
ylabel('speed,round per second')
figure(2);clf
plot(dat1(1:end-20),acc1,'k.',dat2(1:end-16),acc2,'b.')
xlabel('time,s')
ylabel('acceleration,round per s^2')
figure(3);clf
plot(speed1(1:end-8),acc1,'k.',speed2(1:end-8),acc2,'b.')
xlabel('speed,round per second')
ylabel('acceleration,round per s^2')
