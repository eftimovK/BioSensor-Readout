% Impedance measurement optimization
% by setting the p2p excitation 
% output voltage, the RTIA/CTIA combination, 
% and the calibration resistor.
%
% In accordance with the equations from the manual
% "Optimizing the ADuCM350 for Impedance Conversion"


f = 1e3; % [Hz]

omega = 2*pi*f;

Rs = 1e3;
Rp = 10*1e3;
Cp = 220 * 1e-12;


Z_Cp = -1i * 1 / (omega * Cp);

Z_tot = Rs + 1/(1/Rp + 1/Z_Cp);
% Z_tot = Rs*1 + Z_Cp;

Z_tot_min = abs(Z_tot);
R_cal = Z_tot_min           % [A]

angle(Z_tot) * 180 / pi;


% Maximum voltage swing is 600 mV peak (DC)
% and 30mV (AC)
% set it below or equal to this value:
V_swing_max = 30 / 1e3       % [V]

% Highest signal current into TIA
I_max_tia = V_swing_max / R_cal; % [A]


% Peak voltage at output of TIA 
% (maximum allowed by the ADuCM350) = 750 mV peak.
% 
% constant, i.e. cannot be modified...?
V_peak_tia = 750 / 1000;  % [V]


% Transimpedance resistor
R_tia = V_peak_tia / I_max_tia;

% safety factor to avoid overranging the ADC
R_tia = R_tia / 1.2

% anti-aliasing capacitor
f_bw = 80*1e3; % max. system bandwidth [Hz]

C_tia = 1 / (2*pi*f_bw * R_tia)



