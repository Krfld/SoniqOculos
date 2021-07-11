#include "dsp.h"

#include "i2s.h"
#include "sd.h"
#include "bt.h"
#include "gpio.h"

#define DSP_TAG "DSP"

RTC_DATA_ATTR static int volume = DEFAULT_VOLUME; //* % | Keep value while in deep-sleep
int get_volume()
{
    return volume;
}

RTC_DATA_ATTR static int eq_bass = 0;   //* -2..2 | Keep value while in deep-sleep
RTC_DATA_ATTR static int eq_mid = 0;    //* -2..2 | Keep value while in deep-sleep
RTC_DATA_ATTR static int eq_treble = 0; //* -2..2 | Keep value while in deep-sleep

int get_bass()
{
    return eq_bass;
}
int get_mid()
{
    return eq_mid;
}
int get_treble()
{
    return eq_treble;
}

static SemaphoreHandle_t equalizer_semaphore_handle;

static float *data_left;
static float *data_right;
static float *output_left_low;
static float *output_right_low;
static float *output_left_high;
static float *output_right_high;

// Equalizer

// BASS - Low Shelf
static float e_b_low_shelf_coeffs[5];
static float e_b_low_shelf_w_left[2];
static float e_b_low_shelf_w_right[2];

// MIDD - Notch
static float e_m_notch_coeffs[5];
static float e_m_notch_w_left[2];
static float e_m_notch_w_right[2];

// TREBLE - High Shelf
static float e_t_high_shelf_coeffs[5];
static float e_t_high_w_left[2];
static float e_t_high_w_right[2];

// Crossover

// LPF
static float c_lpf_coeffs[5];
static float c_lpf_w_left[2];
static float c_lpf_w_right[2];

// HPF
static float c_hpf_coeffs[5];
static float c_hpf_w_left[2];
static float c_hpf_w_right[2];

void dsp_init()
{
    if (!PROCESSING)
        return;

    equalizer_semaphore_handle = xSemaphoreCreateMutex();
    if (!equalizer_semaphore_handle)
        ESP_LOGE(DSP_TAG, "Error creating semaphore");

    // Equalizer
    dsps_biquad_gen_lowShelf_f32(e_b_low_shelf_coeffs, EQUALIZER_LOW_SHELF_FREQUENCY / SAMPLE_FREQUENCY, (eq_bass - 2) * EQUALIZER_GAIN, Q);      // Generate coeffs for BASS
    dsps_biquad_gen_notch_f32(e_m_notch_coeffs, EQUALIZER_NOTCH_FREQUENCY / SAMPLE_FREQUENCY, (eq_mid - 2) * EQUALIZER_GAIN, Q);                  // Generate coeffs for MID
    dsps_biquad_gen_highShelf_f32(e_t_high_shelf_coeffs, EQUALIZER_HIGH_SHELF_FREQUENCY / SAMPLE_FREQUENCY, (eq_treble - 2) * EQUALIZER_GAIN, Q); // Generate coeffs for TREBLE

    // Crossover
    dsps_biquad_gen_lpf_f32(c_lpf_coeffs, CROSSOVER_FREQUENCY / SAMPLE_FREQUENCY, Q); // Generate coeffs for LPF
    dsps_biquad_gen_hpf_f32(c_hpf_coeffs, CROSSOVER_FREQUENCY / SAMPLE_FREQUENCY, Q); // Generate coeffs for HPF

    data_left = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    data_right = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    output_left_low = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    output_right_low = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    output_left_high = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    output_right_high = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
}

void apply_crossover(uint8_t *input, uint8_t *output_low, uint8_t *output_high, size_t *len)
{
    if (!PROCESSING)
        return;

    // Convert to 2 bytes per sample (16 bit)
    // (int16_t *) samples -> [0] - Left | [1] - Right | [2] - Left | [3] - Right ...
    int16_t *input_16 = (int16_t *)input;
    int16_t *output_low_16 = (int16_t *)output_low;
    int16_t *output_high_16 = (int16_t *)output_high;

    int channel_length_16 = *len / 4;

    for (size_t i = 0; i < channel_length_16; i++)
    {
        data_left[i] = input_16[i * 2] / INT16;      // Normalize left
        data_right[i] = input_16[i * 2 + 1] / INT16; // Normalize right
    }

    // LPF
    dsps_biquad_f32(data_left, output_left_low, channel_length_16, c_lpf_coeffs, c_lpf_w_left);    // Left
    dsps_biquad_f32(data_right, output_right_low, channel_length_16, c_lpf_coeffs, c_lpf_w_right); // Right

    // HPF
    dsps_biquad_f32(data_left, output_left_high, channel_length_16, c_hpf_coeffs, c_hpf_w_left);    // Left
    dsps_biquad_f32(data_right, output_right_high, channel_length_16, c_hpf_coeffs, c_hpf_w_right); // Right

    for (size_t i = 0; i < channel_length_16; i++)
    {
        // LPF
        output_low_16[i * 2] = output_left_low[i] * INT16;      // Denormalize left
        output_low_16[i * 2 + 1] = output_right_low[i] * INT16; // Denormalize right

        // HPF
        output_high_16[i * 2] = output_left_high[i] * INT16;      // Denormalize left
        output_high_16[i * 2 + 1] = output_right_high[i] * INT16; // Denormalize right
    }
}

void set_equalizer(int bass, int mid, int treble)
{
    if (!PROCESSING)
        return;

    eq_bass = bass;
    eq_mid = mid;
    eq_treble = treble;

    xSemaphoreTake(equalizer_semaphore_handle, portMAX_DELAY); // Wait to update equalizer

    dsps_biquad_gen_lowShelf_f32(e_b_low_shelf_coeffs, EQUALIZER_LOW_SHELF_FREQUENCY / SAMPLE_FREQUENCY, (eq_bass - 2) * EQUALIZER_GAIN, Q);      // Generate coeffs for BASS
    dsps_biquad_gen_notch_f32(e_m_notch_coeffs, EQUALIZER_NOTCH_FREQUENCY / SAMPLE_FREQUENCY, (eq_mid - 2) * EQUALIZER_GAIN, Q);                  // Generate coeffs for MID
    dsps_biquad_gen_highShelf_f32(e_t_high_shelf_coeffs, EQUALIZER_HIGH_SHELF_FREQUENCY / SAMPLE_FREQUENCY, (eq_treble - 2) * EQUALIZER_GAIN, Q); // Generate coeffs for TREBLE

    xSemaphoreGive(equalizer_semaphore_handle);

    //spp_send_msg("e %d %d %d", eq_bass, eq_mid, eq_treble);
    spp_send_msg("eb %d", eq_bass);
    spp_send_msg("em %d", eq_mid);
    spp_send_msg("et %d", eq_treble);
}

void apply_equalizer(uint8_t *data, size_t *len)
{
    if (!PROCESSING)
        return;

    // Convert to 2 bytes per sample (16 bit)
    // (int16_t *) samples -> [0] - Left | [1] - Right | [2] - Left | [3] - Right ...
    int16_t *data_16 = (int16_t *)data;

    int channel_length_16 = *len / 4;

    for (size_t i = 0; i < channel_length_16; i++)
    {
        data_left[i] = data_16[i * 2] / INT16;      // Normalize left
        data_right[i] = data_16[i * 2 + 1] / INT16; // Normalize right
    }

    xSemaphoreTake(equalizer_semaphore_handle, portMAX_DELAY); // Wait if equalizer is being updated

    // BASS
    dsps_biquad_f32(data_left, data_left, channel_length_16, e_b_low_shelf_coeffs, e_b_low_shelf_w_left);    // Left
    dsps_biquad_f32(data_right, data_right, channel_length_16, e_b_low_shelf_coeffs, e_b_low_shelf_w_right); // Right

    // MID
    dsps_biquad_f32(data_left, data_left, channel_length_16, e_m_notch_coeffs, e_m_notch_w_left);    // Left
    dsps_biquad_f32(data_right, data_right, channel_length_16, e_m_notch_coeffs, e_m_notch_w_right); // Right

    // TREBLE
    dsps_biquad_f32(data_left, data_left, channel_length_16, e_t_high_shelf_coeffs, e_t_high_w_left);    // Left
    dsps_biquad_f32(data_right, data_right, channel_length_16, e_t_high_shelf_coeffs, e_t_high_w_right); // Right

    xSemaphoreGive(equalizer_semaphore_handle);

    for (size_t i = 0; i < channel_length_16; i++)
    {
        data_16[i * 2] = data_left[i] * INT16;      // Denormalize left
        data_16[i * 2 + 1] = data_right[i] * INT16; // Denormalize right
    }
}

void set_volume(int vol)
{
    vibrate(VIBRATION_DELAY);

    // Limit volume
    if (vol > 100)
        volume = 100;
    else if (vol < 0)
        volume = 0;
    else
        volume = vol;

    spp_send_msg("v %d", volume / 10);

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

    float normalized_volume = volume * MAX_VOLUME / 100.0; // Normalize volume

    for (size_t i = 0; i < len_16; i++)
        data_16[i] *= normalized_volume;
}
