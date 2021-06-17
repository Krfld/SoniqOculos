clc
clear
close all

frequency = 500; % Hz
duration = 5; % Seconds

Fs = 44100; % Sample Rate

t = 0:1 / Fs:duration;
w = 2 * pi * frequency;

out(:, 1) = sin(w*t); % Left
out(:, 2) = sin(w*t); % Right TODO Impulse

audiowrite(strcat('Sine_', num2str(duration), 's_', num2str(frequency), 'Hz.wav'), out, Fs)
% sound(out, Fs)
