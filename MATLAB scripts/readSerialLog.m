%%%
%
% Reads line-by-line and stores the from each in the serial log file.
%
%   Input:
%           filename --- file name and type (full path if required); 
%                             e.g. '..\putty.log'
%           duration --- this refers to the measurement and is only used
%                             for the plot; to be provided in seconds
%
%   Output: 
%           adcCodeArray --- array containing the numbers extracted
%
%%%


function adcCodeArray = readSerialLog(filename, duration)

    % Initialize an empty array to store the numeric values
    adcCodeArray = [];
    
    % Open the text file for reading
    fid = fopen(filename, 'r');
    
    % Check if the file opened successfully
    if fid == -1
        error('Could not open file: %s', filename);
    end
    
    % Read the file line-by-line
    while ~feof(fid)
        line = fgetl(fid); % Read the current line
        
        if ischar(line) % Ensure the line is a character array
            % Try to convert the line to a numeric value
            num = str2double(line); % Convert to double
            
            % Check if the conversion was successful (is a valid number)
            if ~isnan(num)
                % Add the numeric value to the array
                adcCodeArray = [adcCodeArray, num]; % Append to the array
            end
        end
    end
    
    % Close the file
    fclose(fid);
    
    % Plot the data
    numPoints = length(adcCodeArray);
    timeArray = linspace(0, duration, numPoints);
    
    disp('Plotting values read from the file:');
    
    figure;
    plot(timeArray, adcCodeArray, 'b-o'); % Plot with blue circles and lines
    title('RC - amperometric measurement');
    xlabel('Time [s]');
    ylabel('ADC code');
    grid on; % Turn on the grid for easier reading


end