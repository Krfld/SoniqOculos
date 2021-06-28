#include "main.h"

#define DATA_LENGTH 4 * 1024

#define PROCESSING ON
#define FIXED_DATA_LENGTH ON

//* Volume
#define DEFAULT_VOLUME 50  // %
#define VOLUME_INTERVAL 10 // %
#define MAX_VOLUME 0.3

#define SAMPLE_FREQUENCY 44100.0 // Hz

#define EQUALIZER_LOW_SHELF_FREQUENCY 250   // Hz
#define EQUALIZER_NOTCH_FREQUENCY 2500      // Hz
#define EQUALIZER_HIGH_SHELF_FREQUENCY 4000 // Hz

#define CROSSOVER_FREQUENCY 1000 // Hz

#define Q sqrt(2) / 2 // 0.7071067812

void dsp_init();

void apply_crossover(uint8_t *input, uint8_t *output_low, uint8_t *output_high, size_t *len);

void apply_equalizer(uint8_t *data, size_t *len);

void set_volume(int vol);
void volume_up();
void volume_down();

void apply_volume(uint8_t *data, size_t *len);

/**
 * @brief Process data
 * 
 * @param data buffer of samples to process
 * @param len buffer length
 */
void process_data(uint8_t *data, size_t *len);
