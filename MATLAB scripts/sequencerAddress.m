% Returns the two hex digits for a sequencer cmd
% given the address of the register

% Define address of the MMR
hexValue = '0x4008000C';

% Convert the hexadecimal value to a binary string
binStr = dec2bin(hex2dec(hexValue), 32);

% binStr = flip(binStr);

% Extract bits [7:2] from the binary string (1-based indexing in MATLAB)
seq = binStr(25:30);

% Add '1' to the left and '0' to the right of the sequence
newSeq = ['1', seq, '0'];

% Convert the new 8-bit sequence to hexadecimal
newHex = dec2hex(bin2dec(newSeq), 2);

% Display the results
% fprintf('Original Hex Value: %s\n', hexValue);
% fprintf('Extracted Bits [7:2]: %s\n', seq);
% fprintf('New 8-bit Sequence: %s\n', newSeq);
% fprintf('Converted Hex Value: 0x%s\n', newHex);

fprintf('Converted Hex Value: 0x%s\n', newHex);