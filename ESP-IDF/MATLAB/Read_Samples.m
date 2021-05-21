clc
clear
close all

Fs = 44100;
BitsPerSample = 16;
BytesPerSample = (BitsPerSample / 8 * 2);

file = fopen('E:\SAMPLES.TXT');
data = fread(file);

out = zeros(length(data)/BytesPerSample, 2);
for i = 1:length(data) / BytesPerSample
    out(i, 1) = data((i-1)*BytesPerSample+2) * 2^8 + data((i-1)*BytesPerSample+1); % Left

    if (BitsPerSample == 32)
        out(i, 1) = data((i-1)*BytesPerSample+4) * 2^24 + data((i-1)*BytesPerSample+3) * 2^16;
    end

    if (out(i, 1) >= 2^(BitsPerSample - 1))
        out(i, 1) = out(i, 1) - 2^BitsPerSample;
    end

    out(i, 2) = data((i-1)*BytesPerSample+BytesPerSample/2+2) * 2^8 + data((i-1)*BytesPerSample+BytesPerSample/2+1); % Right

    if (BitsPerSample == 32)
        out(i, 1) = data((i-1)*BytesPerSample+BytesPerSample/2+4) * 2^24 + data((i-1)*BytesPerSample+BytesPerSample/2+3) * 2^16;
    end

    if (out(i, 2) >= 2^(BitsPerSample - 1))
        out(i, 2) = out(i, 2) - 2^BitsPerSample;
    end
end
fclose(file);

out = out / 2^15; % Real Volume

plot(out);
sound(out, Fs, BitsPerSample);
