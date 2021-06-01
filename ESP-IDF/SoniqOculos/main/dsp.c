#include "dsp.h"

float lpf_1kHz_delays[FIR_LENGTH];
float lpf_1kHz_coeffs[FIR_LENGTH] = {
    -0.0004628362367, -0.0003387566539, -0.0001964251715, 1.288998919e-05, 0.0003444032045,
    0.0008565972093, 0.001607580343, 0.002651219722, 0.00403327588, 0.005787773058,
    0.007933828048, 0.0104731489, 0.01338837389, 0.01664237492, 0.02017861791,
    0.02392256819, 0.02778413892, 0.03166102991, 0.03544285893, 0.03901581466,
    0.04226767272, 0.0450928323, 0.04739717022, 0.04910241812, 0.05014986917,
    0.05050312728, 0.05014986917, 0.04910241812, 0.04739717022, 0.0450928323,
    0.04226767272, 0.03901581466, 0.03544285893, 0.03166102991, 0.02778413892,
    0.02392256819, 0.02017861791, 0.01664237492, 0.01338837389, 0.0104731489,
    0.007933828048, 0.005787773058, 0.00403327588, 0.002651219722, 0.001607580343,
    0.0008565972093, 0.0003444032045, 1.288998919e-05, -0.0001964251715, -0.0003387566539,
    -0.0004628362367};

float hpf_1kHz_delays[FIR_LENGTH];
float hpf_1kHz_coeffs[FIR_LENGTH] = {
    0.0004158202792, 0.0003043449833, 0.0001764718472, -1.158059422e-05, -0.0003094179265,
    -0.0007695821114, -0.001444278634, -0.002381902654, -0.00362356659, -0.005199837964,
    -0.00712789176, -0.009409262799, -0.01202835236, -0.01495180465, -0.01812882721,
    -0.02149245888, -0.02496176213, -0.02844483033, -0.0318424888, -0.03505250067,
    -0.03797402605, -0.04051220044, -0.04258245602, -0.04411448166, -0.04505552724,
    0.9550995827, -0.04505552724, -0.04411448166, -0.04258245602, -0.04051220044,
    -0.03797402605, -0.03505250067, -0.0318424888, -0.02844483033, -0.02496176213,
    -0.02149245888, -0.01812882721, -0.01495180465, -0.01202835236, -0.009409262799,
    -0.00712789176, -0.005199837964, -0.00362356659, -0.002381902654, -0.001444278634,
    -0.0007695821114, -0.0003094179265, -1.158059422e-05, 0.0001764718472, 0.0003043449833,
    0.0004158202792};

void crossover_init()
{
    //* Init LPF
    fir_lpf_1kHz = (fir_f32_t *)malloc(sizeof(fir_f32_t));
    dsps_fir_init_f32(fir_lpf_1kHz, lpf_1kHz_coeffs, lpf_1kHz_delays, FIR_LENGTH);

    //* Init HPF
    fir_hpf_1kHz = (fir_f32_t *)malloc(sizeof(fir_f32_t));
    dsps_fir_init_f32(fir_hpf_1kHz, hpf_1kHz_coeffs, hpf_1kHz_delays, FIR_LENGTH);
}

void apply_crossover(int16_t *input, int16_t *output_low, int16_t *output_high, size_t *len)
{
    float input_normalized[*len];
    for (size_t i = 0; i < *len; i++)
        input_normalized[i] = input[i] / pow(2, 15); //* Normalize data

    //* LPF
    float output_normalized_lpf[*len];
    dsps_fir_f32_ae32(fir_lpf_1kHz, input_normalized, output_normalized_lpf, *len); //* Process

    for (size_t i = 0; i < *len; i++)
        output_low[i] = output_normalized_lpf[i] * pow(2, 15); //* Denormalize

    //* HPF
    float output_normalized_hpf[*len];
    dsps_fir_f32_ae32(fir_hpf_1kHz, input_normalized, output_normalized_hpf, *len); //* Process

    for (size_t i = 0; i < *len; i++)
        output_high[i] = output_normalized_hpf[i] * pow(2, 15); //* Denormalize
}