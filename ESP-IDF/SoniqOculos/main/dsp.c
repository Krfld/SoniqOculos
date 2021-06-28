#include "dsp.h"

#include "i2s.h"
#include "sd.h"
#include "bt.h"
#include "gpio.h"

#define DSP_TAG "DSP"

RTC_DATA_ATTR static int volume = DEFAULT_VOLUME; //* % | Keep value while in deep-sleep

RTC_DATA_ATTR static int eq_bass = 0;   //* -1,0,1 | Keep value while in deep-sleep
RTC_DATA_ATTR static int eq_mid = 0;    //* -1,0,1 | Keep value while in deep-sleep
RTC_DATA_ATTR static int eq_treble = 0; //* -1,0,1 | Keep value while in deep-sleep

//? Equalizer

//? BASS - Low Shelf
static float e_b_low_shelf_coeffs[5];
static float e_b_low_shelf_w_left[2];
static float e_b_low_shelf_w_right[2];

//? MIDD - Notch
static float e_m_notch_coeffs[5];
static float e_m_notch_w_left[2];
static float e_m_notch_w_right[2];

//? TREBLE - High Shelf
static float e_t_high_shelf_coeffs[5];
static float e_t_high_w_left[2];
static float e_t_high_w_right[2];

//? Crossover

//? LPF
static float c_lpf_coeffs[5];
static float c_lpf_w_left[2];
static float c_lpf_w_right[2];

//? HPF
static float c_hpf_coeffs[5];
static float c_hpf_w_left[2];
static float c_hpf_w_right[2];

void dsp_init()
{
    if (!PROCESSING)
        return;

    //TODO Test negative stopband gain

    //? Equalizer
    dsps_biquad_gen_lowShelf_f32(e_b_low_shelf_coeffs, EQUALIZER_LOW_SHELF_FREQUENCY / SAMPLE_FREQUENCY, eq_bass, Q);      //? BASS
    dsps_biquad_gen_notch_f32(e_m_notch_coeffs, EQUALIZER_NOTCH_FREQUENCY / SAMPLE_FREQUENCY, eq_mid, Q);                  //? MI
    dsps_biquad_gen_highShelf_f32(e_t_high_shelf_coeffs, EQUALIZER_HIGH_SHELF_FREQUENCY / SAMPLE_FREQUENCY, eq_treble, Q); //? TREBLE

    //? Crossover
    dsps_biquad_gen_lpf_f32(c_lpf_coeffs, CROSSOVER_FREQUENCY / SAMPLE_FREQUENCY, Q); //? LPF
    dsps_biquad_gen_hpf_f32(c_hpf_coeffs, CROSSOVER_FREQUENCY / SAMPLE_FREQUENCY, Q); //? HPF
}

void apply_crossover(uint8_t *input, uint8_t *output_low, uint8_t *output_high, size_t *len)
{
    if (!PROCESSING)
        return;

    //? Convert to 2 bytes per sample (16 bit)
    //? (int16_t *) samples -> [0] - Left | [1] - Right | [2] - Left | [3] - Right ...
    int16_t *input_16 = (int16_t *)input;
    int16_t *output_low_16 = (int16_t *)output_low;
    int16_t *output_high_16 = (int16_t *)output_high;

    int channel_length_16 = *len / 4;

    float input_left[channel_length_16];
    float input_right[channel_length_16];
    float output_left_low[channel_length_16];
    float output_right_low[channel_length_16];
    float output_left_high[channel_length_16];
    float output_right_high[channel_length_16];

    for (size_t i = 0; i < channel_length_16; i++)
    {
        input_left[i] = input_16[i * 2] / INT16;      //? Normalize left
        input_right[i] = input_16[i * 2 + 1] / INT16; //? Normalize right
    }

    //? LPF
    dsps_biquad_f32(input_left, output_left_low, channel_length_16, c_lpf_coeffs, c_lpf_w_left);
    dsps_biquad_f32(input_right, output_right_low, channel_length_16, c_lpf_coeffs, c_lpf_w_right);

    //? HPF
    dsps_biquad_f32(input_left, output_left_high, channel_length_16, c_hpf_coeffs, c_hpf_w_left);
    dsps_biquad_f32(input_right, output_right_high, channel_length_16, c_hpf_coeffs, c_hpf_w_right);

    for (size_t i = 0; i < channel_length_16; i++)
    {
        //? LPF
        output_low_16[i * 2] = output_left_low[i] * INT16;      //? Denormalize left
        output_low_16[i * 2 + 1] = output_right_low[i] * INT16; //? Denormalize right

        //? HPF
        output_high_16[i * 2] = output_left_high[i] * INT16;      //? Denormalize left
        output_high_16[i * 2 + 1] = output_right_high[i] * INT16; //? Denormalize right
    }
}

void apply_equalizer(uint8_t *data, size_t *len)
{
    if (!PROCESSING)
        return;

    //? Convert to 2 bytes per sample (16 bit)
    //? (int16_t *) samples -> [0] - Left | [1] - Right | [2] - Left | [3] - Right ...
    int16_t *data_16 = (int16_t *)data;

    int channel_length_16 = *len / 4;

    float data_left[channel_length_16];
    float data_right[channel_length_16];

    for (size_t i = 0; i < channel_length_16; i++)
    {
        data_left[i] = data_16[i * 2] / INT16;      //? Normalize left
        data_right[i] = data_16[i * 2 + 1] / INT16; //? Normalize right
    }

    //* BASS
    dsps_biquad_f32(data_left, data_left, channel_length_16, e_b_low_shelf_coeffs, e_b_low_shelf_w_left);
    dsps_biquad_f32(data_right, data_right, channel_length_16, e_b_low_shelf_coeffs, e_b_low_shelf_w_right);

    for (size_t i = 0; i < channel_length_16; i++)
    {
        data_16[i * 2] = data_left[i] * INT16;      //? Denormalize left
        data_16[i * 2 + 1] = data_right[i] * INT16; //? Denormalize right
    }

    return;

    //* MID
    dsps_biquad_f32(data_left, data_left, channel_length_16, e_m_notch_coeffs, e_m_notch_w_left);
    dsps_biquad_f32(data_right, data_right, channel_length_16, e_m_notch_coeffs, e_m_notch_w_right);

    //* TREBLE
    dsps_biquad_f32(data_left, data_left, channel_length_16, e_t_high_shelf_coeffs, e_t_high_w_left);
    dsps_biquad_f32(data_right, data_right, channel_length_16, e_t_high_shelf_coeffs, e_t_high_w_right);
}

void set_volume(int vol)
{
    //* Limit volume
    if (vol > 100)
        volume = 100;
    else if (vol < 0)
        volume = 0;
    else
        volume = vol;

    spp_send_msg("v %d", volume);

    ESP_LOGI(DSP_TAG, "Volume: %d%%", volume);
}

void volume_up()
{
    set_volume(volume + VOLUME_INTERVAL);
}
void volume_down()
{
    set_volume(volume - VOLUME_INTERVAL);
}

void apply_volume(uint8_t *data, size_t *len)
{
    int16_t *data_16 = (int16_t *)data;
    int len_16 = *len / 2;

    float normalized_volume = volume * MAX_VOLUME / 100.0; //? Normalize volume

    for (size_t i = 0; i < len_16; i++)
        data_16[i] *= normalized_volume;
}

void process_data(uint8_t *data, size_t *len)
{
    //TODO Process data

    i2s_write_data(data, len);

    //sd_write_data(data, len); //! Testing
}