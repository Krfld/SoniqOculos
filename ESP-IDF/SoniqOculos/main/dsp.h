#include "main.h"

#define DATA_LENGTH 4 * 1024

#define PROCESSING OFF
#define FIXED_DATA_LENGTH ON

void apply_crossover(uint8_t *input, uint8_t *output_low, uint8_t *output_high, size_t *len);

void apply_volume(uint8_t *data, size_t *len);

/**
 * @brief Process data
 * 
 * @param data buffer of samples to process
 * @param len buffer length
 */
void process_data(uint8_t *data, size_t *len);
