%% Set path of measurement files, file names, and output folder path

folderPath = "C:\Users\User\Desktop\RWTH-Life\Master\AixSense\DATA team\BioSensor-Readout\Measurement\EIS\Eindhoven";
subfolderName = "BTS";

% get all .txt files names in an array
measFiles = [];
fileList = dir(fullfile(folderPath, subfolderName, '*.txt'));

% sort chronologically (based on date)
[~,idx] = sort([fileList.datenum]);
fileList = fileList(idx);

for i=1:length(fileList)
    measFiles = [measFiles, string(fileList(i).name)];
end

% select particular files only
% measFiles = measFiles(1:end-1);

outputFolder = 'C:\Users\User\Desktop\RWTH-Life\Master\AixSense\DATA team\BioSensor-Readout\Measurement\EIS\Eindhoven\Ref Curve';

% define frequency range to use for the impedance data
% if multiple frequencies then the impedance point is an average
startFreq = 70000;
stopFreq = 60000;

%% Fill in the data for the reference curve

dataPointsNum = length(measFiles);
% store data in vectors
concentration = zeros(dataPointsNum, 1);
impedance = zeros(dataPointsNum, 1);

for i=1:length(measFiles)
    % get the impedance value within the desired freq. range
    filepath = fullfile(folderPath, subfolderName, measFiles(i));
    impedance(i) = impedanceFromMeasFile(filepath, startFreq, stopFreq);

    % get the corresponding concentration from the filename
    concentration(i) = concentrationFromFilename(measFiles(i));

end

%% Sort the data in ascending concentration

[concentration_sorted, sortIdx] = sort(concentration);
concentration = concentration_sorted;
impedance = impedance(sortIdx);

%% Do various fitting of the data

% Linear Fit
p_linear = polyfit(concentration, impedance, 1);
y_linear = polyval(p_linear, concentration);

% Quadratic Fit - first way

p_quadratic = polyfit(concentration, impedance, 2);
y_quadratic = polyval(p_quadratic, concentration);


% % Quadratic Fit - second way
% f_quadratic2 = fit(concentration, impedance, 'poly2');
% y_quadratic2 = feval(f_quadratic2, concentration);

% Logarithmic Fit
fittype_log = fittype('a*log(b*x) + c',...
    'independent',{'x'},...
    'coefficients',{'a','b', 'c'} );
f_log = fit(concentration, impedance, fittype_log, 'StartPoint', [1, 1, 1], 'Lower', [0, 0, 0]);
y_log = feval(f_log, concentration);


%% Do plotting

lineWidth = 3;
markerSize = 12;

figure;

plot(concentration, impedance, 'o', 'Color', 'k', 'MarkerFaceColor', 'k' , 'MarkerSize', markerSize, 'DisplayName', 'measurement data');
hold on;
plot(concentration, y_linear, '--o', 'LineWidth', lineWidth, 'DisplayName', 'Linear Fit');
plot(concentration, y_quadratic, '--o', 'LineWidth', lineWidth, 'DisplayName', 'Quadratic Fit');
plot(concentration, y_log, '--o', 'LineWidth', lineWidth, 'DisplayName', 'Logarithmic Fit');

% plot(concentration, y_quadratic2, 'LineWidth', 2, 'DisplayName', 'Quadratic Fit 2');

plot(concentration, impedance, 'o', 'Color', 'k', 'MarkerFaceColor', 'k' , 'MarkerSize', markerSize, 'DisplayName', 'measurement data');

legend;
xlabel('Creatinine Concentration [uM]');
ylabel('Impedance - real part [Ohm]');
title('Fitting of measured data');

hold off;

%% Calc inverse of data fitting

a = f_log.a;
b = f_log.b;
c = f_log.c;

Z = a * log(b * concentration) + c;
calibrationCurve = @(y) (1/b) * exp((y - c) / a);

% Check whether the calibration gives expected values
crea_known = calibrationCurve( Z )

Zmeas = [135, 133.4, 138.2]; % multiple measurements of unknown impedance
% Estimate creatinine from the unknown impedance Zmeas
crea = calibrationCurve( mean(Zmeas) )

%% Save figure


%% Evaluate R-squared or other metrics to choose the best fit ...?
