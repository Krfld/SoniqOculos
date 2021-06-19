clc
clear
close all

frequency = 500; % Hz
duration = 5; % Seconds
name = 'half_sin_impulse_switch'; %strcat('Left_Sine_', num2str(duration), 's_', num2str(frequency), 'Hz_', 'Right_Empty');

Fs = 44100; % Sample Rate

t = 0:1 / Fs:duration - 1 / Fs;
w = 2 * pi * frequency;
size = length(t);

wave = sin(w*t);

out = zeros(size, 2);

for i = 1:size / 2
    out(i, 1) = wave(i);
    out(i+size/2, 2) = wave(i+size/2);
    if rem(i, Fs) == 0
        out(i, 2) = 1;
    end
    if rem(i+size/2, Fs) == 0
        out(i+size/2, 1) = 1;
    end
end

% out(size/2, 2) = 1;

plot(out);
% xlim([20e3, 20e3 + 1024]);

% audiowrite(strcat(name, '.wav'), out, Fs, 'BitsPerSample', 16)
% sound(out, Fs)
