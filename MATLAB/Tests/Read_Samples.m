clc
clear
% close all

[n, Fs] = audioread('Sine_0.1s_0.5kHz.wav');

%%
Fs = 44100;
bitsPerSample = 16;

file = fopen('D:\ISEL\PFC\SoniqOculos\MATLAB\Tests\notch_-12dB_0.7Q.TXT');
file_data = fread(file);

if bitsPerSample == 16
    data = file_data;
    disp('Using  Speakers & Bone Conductors');
elseif bitsPerSample == 32 % Convert 32 bit to 16 bits (ignore bytes 1 and 2)
    disp('Using  Microphones');
    data = zeros(length(file_data)/2, 1);
    for i = 1:2:length(file_data) / 2
        data(i) = file_data((i-1)*2+3);
        data(i+1) = file_data((i-1)*2+4);
    end
end

out = zeros(length(data)/4, 2);
for i = 1:length(data) / 4
    % Left
    out(i, 1) = data((i-1)*4+2) * 2^8 + data((i-1)*4+1) * 2^0;

    % Convert unsigned to signed
    if (out(i, 1) >= 2^15)
        out(i, 1) = out(i, 1) - 2^16;
    end

    % Right
    out(i, 2) = data((i-1)*4+4) * 2^8 + data((i-1)*4+3) * 2^0;

    % Convert unsigned to signed
    if (out(i, 2) >= 2^15)
        out(i, 2) = out(i, 2) - 2^16;
    end
end

fclose(file);

out = out / 2^15; % Normalize

figure, plot(out); % Time
figure, plot(abs(fft(out))), xlim([0, Fs/2]); % Frequency
% sound(out, Fs, 16);
