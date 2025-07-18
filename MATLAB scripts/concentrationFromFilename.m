function [concentration] = concentrationFromFilename(filename)

% Returns concentration of creatinine from the string of file name.
%   Motivation: measurement files' names contained the creatinine concentration
%   ex. "ISF_30uM_EIS5" or "100crea-PB_EIS2" will return 30 or 100

% Approach: return the first number from the left found in the string;
% this means, if the file is named "1.Measurement-30uM-PB" will not return
% the correct value.

numbers = regexp(filename, '\d+', 'match');  % Find all numbers in the string
concentration = str2double(numbers{1});  % Convert the first matched number to double

end