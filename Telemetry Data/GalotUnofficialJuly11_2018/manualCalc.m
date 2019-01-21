laps = 7;

mins = 33;
secs = 52;

h2totalizer = .6525;
h2totalizer = .692


distance = laps * 6388.10 / 3.28084;
secs = secs + mins * 60;

litersEqOld = h2totalizer * 119.93 / (42.9 * 0.74262);
litersEqNew = h2totalizer * 119.93 / (42.9 * 0.7646);

kmPerLiter = distance / litersEqNew;
avgSpeed = distance ./ secs

%litersEqOld ./ 1000 ./ distance * 100e3

kmPerLiter / 5385

h2eff = 0.59;

mipkwh = (distance / 1609) / (h2totalizer * 119.93e3 / 3.6e6) / h2eff;
