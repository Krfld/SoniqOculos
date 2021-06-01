#include "main.h"

#define FIR_LENGTH 51

fir_f32_t *fir_lpf_1kHz;
fir_f32_t *fir_hpf_1kHz;

void crossover_init();

void apply_crossover(int16_t *input, int16_t *output_low, int16_t *output_high, size_t *len);