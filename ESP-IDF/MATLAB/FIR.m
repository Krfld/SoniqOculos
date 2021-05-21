clc
clear
close all

Fs = 44100;
Fn = Fs/2;

Fc = 1000;

Wn = Fc/Fn

b = fir1(127, Wn, 'low');

freqz(b)
