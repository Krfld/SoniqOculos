#include "dsp.h"

#include "i2s.h"
#include "sd.h"
#include "bt.h"
#include "gpio.h"

#define DSP_TAG "DSP"

/*static fir_f32_t *fir_lpf_1kHz;
static fir_f32_t *fir_hpf_1kHz;

static int lpf_length = 26;
static float lpf_left_delay[26];
static float lpf_right_delay[26];
static float lpf_1kHz_coeffs[26] = {
    0.01106905565, 0.01458931901, 0.0187117774, 0.02338849567, 0.02852285653,
    0.03396807611, 0.03953079507, 0.04498021305, 0.05006245896, 0.05451915786,
    0.05810841173, 0.06062588096, 0.06192350388, 0.06192350388, 0.06062588096,
    0.05810841173, 0.05451915786, 0.05006245896, 0.04498021305, 0.03953079507,
    0.03396807611, 0.02852285653, 0.02338849567, 0.0187117774, 0.01458931901,
    0.01106905565};

static int hpf_length = 26;
static float hpf_left_delay[26];
static float hpf_right_delay[26];
static float hpf_1kHz_coeffs[26] = {
    0.001728184172, 0.0007238018443, -0.001026390702, -0.003779287217, -0.007852439769,
    -0.01365482341, -0.02175555564, -0.03304109722, -0.04909909517, -0.07327919453,
    -0.1143176854, -0.2044693679, -0.6351770163, 0.6351770163, 0.2044693679,
    0.1143176854, 0.07327919453, 0.04909909517, 0.03304109722, 0.02175555564,
    0.01365482341, 0.007852439769, 0.003779287217, 0.001026390702, -0.0007238018443,
    -0.001728184172};

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

    //? Init LPF
    fir_lpf_1kHz = (fir_f32_t *)pvPortMalloc(sizeof(fir_f32_t));
    dsps_fir_init_f32(fir_lpf_1kHz, lpf_1kHz_coeffs, lpf_left_delay, lpf_length);

    //? Init HPF
    fir_hpf_1kHz = (fir_f32_t *)pvPortMalloc(sizeof(fir_f32_t));
    dsps_fir_init_f32(fir_hpf_1kHz, hpf_1kHz_coeffs, hpf_left_delay, hpf_length);

    //? Allocate float arrays for each channel
    input_left = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    input_right = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    output_low_left = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    output_low_right = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    output_high_left = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
    output_high_right = (float *)pvPortMalloc(DATA_LENGTH / 4 * sizeof(float));
}

void apply_crossover(uint8_t *input, uint8_t *output_low, uint8_t *output_high, size_t *len)
{
    if (!PROCESSING)
        return;

    int64_t start;
    if (DSP_DEBUG)
        start = esp_timer_get_time();

    //? Convert to 2 bytes per sample (16 bit)
    //? (int16_t *) samples -> [0] - Left | [1] - Right | [2] - Left | [3] - Right ...
    int16_t *input_16 = (int16_t *)input;
    int16_t *output_low_16 = (int16_t *)output_low;
    int16_t *output_high_16 = (int16_t *)output_high;

    size_t channel_length_16 = *len / 4; //? Number of samples per channel

    for (size_t i = 0; i < channel_length_16; i++)
    {
        input_left[i] = input_16[i * 2] / 1<<15;      //? Normalize left
        input_right[i] = input_16[i * 2 + 1] / 1<<15; //? Normalize right
    }

    //? LPF
    fir_lpf_1kHz->delay = lpf_left_delay;
    dsps_fir_f32(fir_lpf_1kHz, input_left, output_low_left, channel_length_16); //? Process left
    fir_lpf_1kHz->delay = lpf_right_delay;
    dsps_fir_f32(fir_lpf_1kHz, input_right, output_low_right, channel_length_16); //? Process right

    //! Something wrong with hpf

    //? HPF
    fir_hpf_1kHz->delay = hpf_left_delay;
    dsps_fir_f32(fir_hpf_1kHz, input_left, output_high_left, channel_length_16); //? Process left
    fir_hpf_1kHz->delay = hpf_right_delay;
    dsps_fir_f32(fir_hpf_1kHz, input_right, output_high_right, channel_length_16); //? Process right

    for (size_t i = 0; i < channel_length_16; i++)
    {
        //? Low
        output_low_16[i * 2] = output_low_left[i] * 1<<15;      //? Denormalize left
        output_low_16[i * 2 + 1] = output_low_right[i] * 1<<15; //? Denormalize right

        //? High
        output_high_16[i * 2] = output_high_left[i] * 1<<15;      //? Denormalize left
        output_high_16[i * 2 + 1] = output_high_right[i] * 1<<15; //? Denormalize right
    }

    if (DSP_DEBUG)
        ESP_LOGI(DSP_TAG, "Crossover delay: %lldus", esp_timer_get_time() - start);
}*/

#define FIR_LENGTH 26

static int fir_pos;
static float left_delay[FIR_LENGTH];
static float right_delay[FIR_LENGTH];

//? fc = 1kHz
static float lpf_coeffs[FIR_LENGTH] = {
    0.01106905565, 0.01458931901, 0.0187117774, 0.02338849567, 0.02852285653,
    0.03396807611, 0.03953079507, 0.04498021305, 0.05006245896, 0.05451915786,
    0.05810841173, 0.06062588096, 0.06192350388, 0.06192350388, 0.06062588096,
    0.05810841173, 0.05451915786, 0.05006245896, 0.04498021305, 0.03953079507,
    0.03396807611, 0.02852285653, 0.02338849567, 0.0187117774, 0.01458931901,
    0.01106905565};
static float hpf_coeffs[FIR_LENGTH] = {
    0.001728184172, 0.0007238018443, -0.001026390702, -0.003779287217, -0.007852439769,
    -0.01365482341, -0.02175555564, -0.03304109722, -0.04909909517, -0.07327919453,
    -0.1143176854, -0.2044693679, -0.6351770163, 0.6351770163, 0.2044693679,
    0.1143176854, 0.07327919453, 0.04909909517, 0.03304109722, 0.02175555564,
    0.01365482341, 0.007852439769, 0.003779287217, 0.001026390702, -0.0007238018443,
    -0.001728184172};

void apply_crossover(uint8_t *input, uint8_t *output_low, uint8_t *output_high, size_t *len)
{
    if (!PROCESSING)
        return;

    //? Convert to 2 bytes per sample (16 bit)
    //? (int16_t *) samples -> [0] - Left | [1] - Right | [2] - Left | [3] - Right ...
    int16_t *input_16 = (int16_t *)input;
    int16_t *output_low_16 = (int16_t *)output_low;
    int16_t *output_high_16 = (int16_t *)output_high;

    int channel_len_16 = *len / 4;

    //? dsps_fir_f32 adapted to optimize performance
    for (size_t i = 0; i < channel_len_16; i++)
    {
        float lpf_left_acc = 0;
        float lpf_right_acc = 0;
        float hpf_left_acc = 0;
        float hpf_right_acc = 0;

        int coeff_pos = FIR_LENGTH - 1;

        left_delay[fir_pos] = input_16[i * 2] * pow(2, 15);      //? Normalize left
        right_delay[fir_pos] = input_16[i * 2 + 1] * pow(2, 15); //? Normalize right
        fir_pos++;

        if (fir_pos >= FIR_LENGTH)
            fir_pos = 0;

        for (int n = fir_pos; n < FIR_LENGTH; n++)
        {
            //? LPF
            lpf_left_acc += lpf_coeffs[coeff_pos] * left_delay[n];
            lpf_right_acc += lpf_coeffs[coeff_pos] * right_delay[n];

            //? HPF
            hpf_left_acc += hpf_coeffs[coeff_pos] * left_delay[n];
            hpf_right_acc += hpf_coeffs[coeff_pos] * right_delay[n];

            coeff_pos--;
        }
        for (int n = 0; n < fir_pos; n++)
        {
            //? LPF
            lpf_left_acc += lpf_coeffs[coeff_pos] * left_delay[n];
            lpf_right_acc += lpf_coeffs[coeff_pos] * right_delay[n];

            //? HPF
            hpf_left_acc += hpf_coeffs[coeff_pos] * left_delay[n];
            hpf_right_acc += hpf_coeffs[coeff_pos] * right_delay[n];

            coeff_pos--;
        }

        //* LPF
        output_low_16[i * 2] = lpf_left_acc / pow(2, 15);      //? Denormalize left
        output_low_16[i * 2 + 1] = lpf_right_acc / pow(2, 15); //? Denormalize right

        //* HPF
        output_high_16[i * 2] = hpf_left_acc / pow(2, 15);      //? Denormalize left
        output_high_16[i * 2 + 1] = hpf_right_acc / pow(2, 15); //? Denormalize right
    }
}

void apply_volume(uint8_t *data, size_t *len)
{
    int16_t *data_16 = (int16_t *)data;
    int len_16 = *len / 2;

    float volume = get_volume() / 100.0; //* Normalize volume

    for (size_t i = 0; i < len_16; i++)
        data_16[i] *= volume;
}

void process_data(uint8_t *data, size_t *len)
{
    //TODO Process data

    i2s_write_data(data, len);

    //sd_write_data(data, len); //! Testing
}