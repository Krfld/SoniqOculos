#include "main.h"

#define SPEAKERS_I2S_NUM I2S_NUM_0
#define MICROPHONES_I2S_NUM I2S_NUM_0
#define BONE_CONDUCTORS_I2S_NUM I2S_NUM_1

#define SPEAKERS_WS_PIN 18
#define SPEAKERS_BCK_PIN 5
#define SPEAKERS_DATA_PIN 17 // TX2

#define MICROPHONES_WS_PIN 32
#define MICROPHONES_BCK_PIN 27
#define MICROPHONES_DATA_PIN 33

#define BONE_CONDUCTORS_WS_PIN 3
#define BONE_CONDUCTORS_BCK_PIN 21
#define BONE_CONDUCTORS_DATA_PIN 19

#define DMA_BUFFER_COUNT 4
#define DMA_BUFFER_LEN 1024

void speakers_init();
void speakers_deinit();

void microphones_init();
void microphones_deinit();

void bone_conductors_init();
void bone_conductors_deinit();

void set_mode(int mode);

void i2s_write_data(uint8_t *data, size_t *len);
