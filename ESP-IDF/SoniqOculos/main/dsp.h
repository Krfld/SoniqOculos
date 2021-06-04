#include "main.h"

#define DSP_TAG "DSP"

#define DATA_LENGTH 4 * 1024

#define FIR_LENGTH 51

fir_f32_t *fir_lpf_1kHz;
fir_f32_t *fir_hpf_1kHz;

void crossover_init();

void apply_crossover(uint8_t *input, uint8_t *output_low, uint8_t *output_high, size_t *len);

/**
 * @brief Process data
 * 
 * @param data buffer of samples to process
 * @param len buffer length
 */
void process_data(uint8_t *data, size_t *len);
