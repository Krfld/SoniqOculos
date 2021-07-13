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

/**
 * @brief Get volume
 * 
 * @return int volume in %
 */
int get_volume();

/**
 * @brief Get bass value
 * 
 * @return int value (-2..2)
 */
int get_bass();
/**
 * @brief Get mid value
 * 
 * @return int value (-2..2)
 */
int get_mid();
/**
 * @brief Get treble value
 * 
 * @return int value (-2..2)
 */
int get_treble();

/**
 * @brief Init DSP variables
 * 
 */
void dsp_init();

/**
 * @brief Apply crossover
 * 
 * @param input data to be processed
 * @param output_low low pass filtered samples
 * @param output_high high pass filtered samples
 * @param len input size
 */
void apply_crossover(uint8_t *input, uint8_t *output_low, uint8_t *output_high, size_t *len);

/**
 * @brief Apply equalizer
 * 
 * @param data data to be equalized
 * @param len data size
 */
void apply_equalizer(uint8_t *data, size_t *len);

/**
 * @brief Set volume
 * 
 * @param vol volume in %
 */
void set_volume(int vol);
/**
 * @brief Increase volume by VOLUME_INTERVAL
 * 
 */
void volume_up();
/**
 * @brief Decrease volume by VOLUME_INTERVAL
 * 
 */
void volume_down();

/**
 * @brief Set equalizer band gains
 * 
 * @param bass bass value (-2..2)
 * @param mid mid value (-2..2)
 * @param treble treble value (-2..2)
 */
void set_equalizer(int bass, int mid, int treble);

/**
 * @brief Apply volume
 * 
 * @param data data to be processed
 * @param len data size
 */
void apply_volume(uint8_t *data, size_t *len);
