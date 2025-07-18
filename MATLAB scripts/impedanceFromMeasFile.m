function [impedance] = impedanceFromMeasFile(filepath, startFreq, stopFreq)

% Returns impedance value from the measurement data within a given frequency range.
%   If more than one frequency is within this range, the mean impedance is
%   calculated as return value.
    
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

    % store the data only within the freq. range
    startIdx = find( Freq <= startFreq);
    stopIdx = find( Freq <= stopFreq);
    if (isempty(startIdx) || isempty(stopIdx) )
        error('Frequency range invalid for the measurement file : %s', filepath);
    end

    startIdx = startIdx(1);
    stopIdx = stopIdx(1);

    Re = data(startIdx:stopIdx, 1);

    impedance = mean(Re);
    
end