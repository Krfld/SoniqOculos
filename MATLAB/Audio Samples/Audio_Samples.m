% clc
% clear
% close all

Fs = 44100; % Sample Rate
frequency = 1; % kHz
duration = 5; % Seconds
name = strcat('Sine_', num2str(duration), 's_', num2str(frequency), 'kHz');

t = 0:1 / Fs:duration - 1 / Fs;
w = 2 * pi * frequency * 1e3;
size = length(t);

% Sin
% wave = sin(w*t);
% out(:, 1) = wave;
% out(:, 2) = wave;

% White noise
out = rand(size, 2) * 2 - 1;

% Variations
% out = zeros(size, 2);

% for i = 1:size / 2
%     out(i, 1) = wave(i);
%     out(i+size/2, 2) = wave(i+size/2);
%     if rem(i, Fs) == 0
%         out(i, 2) = 1;
%     end
%     if rem(i+size/2, Fs) == 0
%         out(i+size/2, 1) = 1;
%     end
% end

% out(size/2, 2) = 1;

figure
plot(out);
% xlim([20e3, 20e3 + 1024]);

% audiowrite(strcat(name, '.wav'), out, Fs, 'BitsPerSample', 16);
% sound(out, Fs)
