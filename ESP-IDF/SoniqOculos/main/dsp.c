#include "dsp.h"

#include "i2s.h"
#include "sd.h"
#include "bt.h"
#include "gpio.h"

#define DSP_TAG "DSP"

static fir_f32_t *fir_lpf_1kHz;
static fir_f32_t *fir_hpf_1kHz;

static int lpf_length = 33;
static float *lpf_left_delays;
static float *lpf_right_delays;
static float lpf_32_1kHz_coeffs[33] = {
    0.001760181854, 0.002317697275, 0.003472622717, 0.005398186389, 0.008218253031,
    0.01199117769, 0.0166987069, 0.02224117145, 0.02843963914, 0.03504508361,
    0.04175399989, 0.04822924733, 0.05412446707, 0.05911002681, 0.06289824098,
    0.06526575983, 0.06607107818, 0.06526575983, 0.06289824098, 0.05911002681,
    0.05412446707, 0.04822924733, 0.04175399989, 0.03504508361, 0.02843963914,
    0.02224117145, 0.0166987069, 0.01199117769, 0.008218253031, 0.005398186389,
    0.003472622717, 0.002317697275, 0.001760181854};

static int hpf_length = 33;
static float *hpf_left_delays;
static float *hpf_right_delays;
static float hpf_32_1kHz_coeffs[33] = {
    -0.001209530281, -0.00159263378, -0.002386254724, -0.003709428944, -0.005647272337,
    -0.008239882998, -0.01147471834, -0.0152832903, -0.01954264194, -0.02408165485,
    -0.02869176678, -0.03314131126, -0.03719228134, -0.04061817005, -0.04322129115,
    -0.04484815523, 0.9557024837, -0.04484815523, -0.04322129115, -0.04061817005,
    -0.03719228134, -0.03314131126, -0.02869176678, -0.02408165485, -0.01954264194,
    -0.0152832903, -0.01147471834, -0.008239882998, -0.005647272337, -0.003709428944,
    -0.002386254724, -0.00159263378, -0.001209530281};

static float *input_left;
static float *input_right;
static float *output_low_left;
static float *output_low_right;
static float *output_high_left;
static float *output_high_right;

void crossover_init()
{
    if (!PROCESSING)
        return;

    lpf_left_delays = (float *)malloc(lpf_length * sizeof(float));
    lpf_right_delays = (float *)malloc(lpf_length * sizeof(float));
    hpf_left_delays = (float *)malloc(hpf_length * sizeof(float));
    hpf_right_delays = (float *)malloc(hpf_length * sizeof(float));

    //* Init LPF
    fir_lpf_1kHz = (fir_f32_t *)pvPortMalloc(sizeof(fir_f32_t));
    dsps_fir_init_f32(fir_lpf_1kHz, lpf_32_1kHz_coeffs, lpf_left_delays, lpf_length);

    //* Init HPF
    fir_hpf_1kHz = (fir_f32_t *)pvPortMalloc(sizeof(fir_f32_t));
    dsps_fir_init_f32(fir_hpf_1kHz, hpf_32_1kHz_coeffs, hpf_left_delays, hpf_length);

    input_left = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    input_right = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    output_low_left = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    output_low_right = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    output_high_left = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    output_high_right = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
}

void apply_crossover(uint8_t *input, uint8_t *output_low, uint8_t *output_high, size_t *len)
{
    int64_t time;
    if (DSP_DEBUG)
        time = esp_timer_get_time();

    //* Convert to 2 bytes per sample (16 bit)
    //* (int16_t *) data -> [0] - Left | [1] - Right | [2] - Left | [3] - Right ...
    int16_t *input_16 = (int16_t *)input;
    int16_t *output_low_16 = (int16_t *)output_low;
    int16_t *output_high_16 = (int16_t *)output_high;

    size_t channel_length_16 = *len / 4; //* Number of samples per channel

    for (size_t i = 0; i < channel_length_16; i++)
    {
        input_left[i] = input_16[i * 2] / pow(2, 15);      //* Normalize left
        input_right[i] = input_16[i * 2 + 1] / pow(2, 15); //* Normalize right
    }

    //* LPF
    fir_lpf_1kHz->delay = lpf_left_delays;
    dsps_fir_f32_ae32(fir_lpf_1kHz, input_left, output_low_left, channel_length_16); //* Process left
    fir_lpf_1kHz->delay = lpf_right_delays;
    dsps_fir_f32_ae32(fir_lpf_1kHz, input_right, output_low_right, channel_length_16); //* Process right

    //TODO Test swap lpf with hpf

    //* HPF
    fir_hpf_1kHz->delay = hpf_left_delays;
    dsps_fir_f32_ae32(fir_hpf_1kHz, input_left, output_high_left, channel_length_16); //* Process left
    fir_hpf_1kHz->delay = hpf_right_delays;
    dsps_fir_f32_ae32(fir_hpf_1kHz, input_right, output_high_right, channel_length_16); //* Process right

    for (size_t i = 0; i < channel_length_16; i++)
    {
        //* Low
        output_low_16[i * 2] = output_low_left[i] * pow(2, 15);      //* Denormalize left
        output_low_16[i * 2 + 1] = output_low_right[i] * pow(2, 15); //* Denormalize right

        //* High
        output_high_16[i * 2] = output_high_left[i] * pow(2, 15);      //* Denormalize left
        output_high_16[i * 2 + 1] = output_high_right[i] * pow(2, 15); //* Denormalize right
    }

    if (DSP_DEBUG)
        ESP_LOGE(DSP_TAG, "Crossover delay: %lld us", esp_timer_get_time() - time);
}

void apply_volume(uint8_t *data, size_t *len)
{
    int16_t *data_16 = (int16_t *)data;

    for (size_t i = 0; i < *len / 2; i++)
        data_16[i] *= get_volume() / 100.0; //* Normalize volume
}

void process_data(uint8_t *data, size_t *len)
{
    //TODO Process data

    i2s_write_data(data, len);

    //sd_write_data(data, len); //! Testing
}
