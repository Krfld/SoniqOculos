#include "dsp.h"

#include "i2s.h"
#include "sd.h"
#include "bt.h"

static float lpf_1kHz_delays[FIR_LENGTH];
static float lpf_1kHz_coeffs[FIR_LENGTH] = {
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

static float hpf_1kHz_delays[FIR_LENGTH];
static float hpf_1kHz_coeffs[FIR_LENGTH] = {
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
    fir_lpf_1kHz = (fir_f32_t *)pvPortMalloc(sizeof(fir_f32_t));
    dsps_fir_init_f32(fir_lpf_1kHz, lpf_1kHz_coeffs, lpf_1kHz_delays, FIR_LENGTH);

    //* Init HPF
    fir_hpf_1kHz = (fir_f32_t *)pvPortMalloc(sizeof(fir_f32_t));
    dsps_fir_init_f32(fir_hpf_1kHz, hpf_1kHz_coeffs, hpf_1kHz_delays, FIR_LENGTH);
}

void apply_crossover(uint8_t *input, uint8_t *output_low, uint8_t *output_high, size_t *len)
{
    //* Convert to 2 bytes per sample (16 bit)

    int16_t *input_16 = (int16_t *)input;
    int16_t *output_low_16 = (int16_t *)output_low;
    int16_t *output_high_16 = (int16_t *)output_high;
    size_t channel_length_16 = *len / 4;

    //* Normalize

    float *input_left = (float *)pvPortMalloc(channel_length_16 * sizeof(float));
    float *input_right = (float *)pvPortMalloc(channel_length_16 * sizeof(float));

    for (size_t i = 0; i < channel_length_16; i++)
    {
        input_left[i] = input_16[i * 2] / pow(2, 15);      //* Normalize left
        input_right[i] = input_16[i * 2 + 1] / pow(2, 15); //* Normalize right
    }

    //* LPF

    if (i2s_get_device_state(BONE_CONDUCTORS_I2S_NUM))
    {
        float *output_low_left = (float *)pvPortMalloc(channel_length_16 * sizeof(float));
        float *output_low_right = (float *)pvPortMalloc(channel_length_16 * sizeof(float));
        dsps_fir_f32_ae32(fir_lpf_1kHz, input_left, output_low_left, channel_length_16);   //* Process left
        dsps_fir_f32_ae32(fir_lpf_1kHz, input_right, output_low_right, channel_length_16); //* Process right

        for (size_t i = 0; i < channel_length_16; i++)
        {
            output_low_16[i * 2] = output_low_left[i] * pow(2, 15);      //* Denormalize left
            output_low_16[i * 2 + 1] = output_low_right[i] * pow(2, 15); //* Denormalize right
        }

        vPortFree(output_low_left);
        vPortFree(output_low_right);
    }

    //* HPF

    if (i2s_get_device_state(SPEAKERS_MICROPHONES_I2S_NUM))
    {
        float *output_high_left = (float *)pvPortMalloc(channel_length_16 * sizeof(float));
        float *output_high_right = (float *)pvPortMalloc(channel_length_16 * sizeof(float));
        dsps_fir_f32_ae32(fir_hpf_1kHz, input_left, output_high_left, channel_length_16);   //* Process left
        dsps_fir_f32_ae32(fir_hpf_1kHz, input_right, output_high_right, channel_length_16); //* Process right

        for (size_t i = 0; i < channel_length_16; i++)
        {
            output_high_16[i * 2] = output_high_left[i] * pow(2, 15);      //* Denormalize left
            output_high_16[i * 2 + 1] = output_high_right[i] * pow(2, 15); //* Denormalize right
        }

        vPortFree(output_high_left);
        vPortFree(output_high_right);
    }

    vPortFree(input_left);
    vPortFree(input_right);
}

void process_data(uint8_t *data, size_t *len) //* (int16_t *) data -> [0] - Left | [1] - Right | [2] - Left | [3] - Right ...
{
    //TODO Process data

    i2s_write_data(data, len);

    //sd_write_data(data, len); //! Testing
}