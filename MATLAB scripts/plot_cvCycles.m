cycleDuration = 3.2;    % [s]
vL1 = -0.4; % voltage level 1
vL2 = 0.8; % voltage level 2
numCycles = 10;
numSamples = 3198;    % expected number of samples in each cycle for the duration of 3.2sec
vector = ff_chip1_10cycles_test1;  % replace vector with the actual data

numSamples = floor( length(vector) / numCycles );   % chip2_test2 had less samples that expected
vector = vector(1:numSamples*numCycles);

% Reshape the vector so that columns contain samples of each cycle
matrix = reshape(vector, [numSamples, numCycles]);

% Define the x-range
timeArray = linspace(0, cycleDuration, numSamples);

halfNumSamples = floor(numSamples / 2);
firstHalf = linspace(vL1, vL2, halfNumSamples);
secondHalf = linspace(vL2, vL1, numSamples - halfNumSamples);
voltageArray = [firstHalf, secondHalf];

% Plot in a new figure
figure;
hold on;
colors = copper(numCycles);  % choose a colormap (see doc for more options)

for i = 1:numCycles
    plot(voltageArray, matrix(:, i), 'Color', colors(i, :), 'LineWidth', 3, 'LineStyle', '-');
end

% plot constant line denoting the zero
yline(I_zero, '-', 'zero current')

hold off;
title(['CV of ferro-ferricyanide']);
xlabel('Voltage [V]');
ylabel('ADC code');
legend(arrayfun(@(i) sprintf('Cycle %d', i), 1:numCycles, 'UniformOutput', false) );
