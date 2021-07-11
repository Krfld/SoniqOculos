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

#define EQUALIZER_GAIN 3 // dB or 4

#define CROSSOVER_FREQUENCY 1000 // Hz

#define Q sqrt(2) / 2 // 0.7071067812

int get_volume();

int get_bass();
int get_mid();
int get_treble();

void dsp_init();

void apply_crossover(uint8_t *input, uint8_t *output_low, uint8_t *output_high, size_t *len);

void apply_equalizer(uint8_t *data, size_t *len);

void set_volume(int vol);
void volume_up();
void volume_down();

void set_equalizer(int bass, int mid, int treble);

void apply_volume(uint8_t *data, size_t *len);
