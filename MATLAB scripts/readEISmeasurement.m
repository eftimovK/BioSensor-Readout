% 
% Nyquist Plot from .txt files, with data in the format:
% Re Im Freq
% 
% Specify file names as an array of strings (measFiles)
% 
% Restriction : all measurement files need to be in the same folder!
    
%% Set path of measurement files, file names, and output folder path

folderPath = "C:\...\BioSensor-Readout\23.08";
subfolderName = "chip2-session5";

% set files to read manually
measFiles = ["200crea-PB_EIS4.txt", ...
                    "100crea-PB_EIS6.txt" ];

% OR ...
% get all .txt files names in an array
measFiles = [];
fileList = dir(fullfile(folderPath, subfolderName, '*.txt'));
% sort chronologically (based on date)
[~,idx] = sort([fileList.datenum]);
fileList = fileList(idx);

for i=1:length(fileList)
    measFiles = [measFiles, string(fileList(i).name)];
end
% measFiles = measFiles(1:end-1);

outputFolder = 'C:\...\BioSensor-Readout\23.08\matlab output';
outputFolderFigures = fullfile(outputFolder, 'figs');

startFreq = 70000;
stopFreq = 10000;
labelFreqBool = 0;

plotTitle = strcat("EIS --- multiple crea conc., ", subfolderName, ", 10mV, ", ...
                            num2str(startFreq/1000), "kHz-", num2str(stopFreq/1000), "kHz");

%% Open file, read data & plot

figure;

for m = 1:length(measFiles)

    % Specify the path to the data file
    filepath = fullfile(folderPath, subfolderName, measFiles(m));
    
    % Open the file for reading
    fileID = fopen(filepath, 'r');
    if fileID == -1
        error('Cannot open the file: %s', filepath);
    end
    
    % Read the file line by line
    data = [];
    line = fgetl(fileID);
    while ischar(line)
        % Replace commas with dots
        line = strrep(line, ',', '.');
        % Split the line into parts
        parts = sscanf(line, '%f %f %f');
        % Append the data
        data = [data; parts'];
        line = fgetl(fileID);
    end
    
    % Close the file
    fclose(fileID);
    
    % Extract Re, Im, and Freq columns
    Re = data(1:end, 1);
    Im = data(1:end, 2);
    Freq = data(1:end, 3);

    % Plot only a certain range of frequency
    startIdx = find( Freq <= startFreq);
    if (isempty(startIdx))
        startIdx = 1;
    else
        startIdx = startIdx(1);
    end
    stopIdx = find( Freq <= stopFreq);
    if (isempty(stopIdx))
        stopIdx = length(Freq);
    else
        stopIdx = stopIdx(1);
    end

    Re = data(startIdx:stopIdx, 1);
    Im = data(startIdx:stopIdx, 2);
    Freq = data(startIdx:stopIdx, 3);
    
    % Label the Nyquist plot
    if (m==1)
        xlabel('Real Part (Re)');
        ylabel('Imaginary Part (Im)');
        title(plotTitle);
        grid on;
        axis equal;
        hold on
    end

    plot(Re, Im, 'o-', 'LineWidth', 1);
    
    % Optionally, label points with corresponding frequencies
    if (labelFreqBool)
        for i = 1:2:length(Freq)
            text(Re(i), Im(i), sprintf('%.2f', Freq(i)), 'VerticalAlignment', 'bottom', 'HorizontalAlignment', 'right');
        end
    end

end

legend(erase(measFiles, ".txt"));

%% Save figure plot as .png and .fig

% Create the folder path where the figure will be saved
if ~exist(outputFolder, 'dir')
    error('Save folder for .png not found.');
end

% Save the figure as .png
saveas(gcf, fullfile(outputFolder, strcat(plotTitle, ".png")));

figFolder = fullfile(outputFolder, 'figs');
if ~exist(figFolder, 'dir')
    error('Save folder for .fig not found.');
end

% Save the figure as FIG in the 'figs' subfolder
saveas(gcf, fullfile(figFolder, strcat(plotTitle, ".fig")));

disp('Saved plot successfully.');

% Close the figure
% close(gcf);