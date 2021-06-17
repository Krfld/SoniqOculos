clc
clear
close all

frequency = 500; % Hz
duration = 5; % Seconds
name = 'empty'; %strcat('Left_Sine_', num2str(duration), 's_', num2str(frequency), 'Hz_', 'Right_Empty');

Fs = 44100; % Sample Rate

t = 0:1 / Fs:duration - 1 / Fs;
w = 2 * pi * frequency;

out(:, 1) = zeros(length(t), 1); %sin(w*t); % Left
out(:, 2) = zeros(length(t), 1); % Right
% out(Fs*duration/2, 2) = 1;

audiowrite(strcat(name, '.wav'), out, Fs, 'BitsPerSample', 16)
% sound(out, Fs)
