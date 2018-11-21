track1 = importdata('track1.csv');
track2 = importdata('track2.csv');
track3 = importdata('track3.csv');


clf;
scatter3(track1(:, 1), track1(:, 2), track1(:, 3)); hold on;
scatter3(track2(:, 1), track2(:, 2), track2(:, 3)); hold on;
scatter3(track3(:, 1), track3(:, 2), track3(:, 3)); hold on;