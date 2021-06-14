clc
clear
close all

Fs = 44100;
precision = 'uint8=>int16'; % 'int32' when recorded with microphones

file = fopen('E:\TESTING.TXT');
[data, len] = fread(file, precision);

out = zeros(len/2, 2);
for i = 1:len / 2
    out(i, 1) = data((i-1)*2+1);
    out(i, 2) = data((i-1)*2+2);
end

fclose(file);

out = out / 2^15; % Normalize

plot(out);
% sound(out, Fs, 16);

%%

clc
clear
close all

Fs = 44100;
bitsPerSample = 32;

file = fopen('fir_test_sin_500Hz.TXT');
file_data = fread(file);

if bitsPerSample == 16
    data = file_data;
elseif bitsPerSample == 32 % Convert 32 bit to 16 bits (ignore bytes 1 and 2)
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

plot(out(:,1));
xlim([20e3 20e3+1024]);
sound(out, Fs, 16);

%% Backup

clc
clear
close all

Fs = 44100;
bitsPerSample = 32;

bytesPerSample = (bitsPerSample / 8 * 2);

file = fopen('TESTING.TXT');
data = fread(file);
data_16 = typecast(typecast(data, 'uint8'), 'uint16');

out = zeros(length(data)/bytesPerSample, 2);
for i = 1:length(data) / bytesPerSample
    % Left
    out(i, 1) = data((i-1)*bytesPerSample+2) * 2^8 + data((i-1)*bytesPerSample+1) * 2^0;
    if (bitsPerSample == 32)
        out(i, 1) = data((i-1)*bytesPerSample+4) * 2^24 + data((i-1)*bytesPerSample+3) * 2^16;
    end

    % Convert unsigned to signed
    if (out(i, 1) >= 2^(bitsPerSample - 1))
        out(i, 1) = out(i, 1) - 2^bitsPerSample;
    end

    % Right
    out(i, 2) = data((i-1)*bytesPerSample+bytesPerSample/2+2) * 2^8 + data((i-1)*bytesPerSample+bytesPerSample/2+1) * 2^0;
    if (bitsPerSample == 32)
        out(i, 1) = data((i-1)*bytesPerSample+bytesPerSample/2+4) * 2^24 + data((i-1)*bytesPerSample+bytesPerSample/2+3) * 2^16;
    end

    % Convert unsigned to signed
    if (out(i, 2) >= 2^(bitsPerSample - 1))
        out(i, 2) = out(i, 2) - 2^bitsPerSample;
    end
end

fclose(file);

out = out / 2^(bitsPerSample - 1); % Normalize

plot(out);
% sound(out, Fs, bitsPerSample);
