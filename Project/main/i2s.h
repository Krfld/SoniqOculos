#include "main.h"

#define SPEAKERS_I2S_NUM I2S_NUM_0
#define SPEAKERS_WS_PIN 18
#define SPEAKERS_BCK_PIN 5
#define SPEAKERS_DATA_PIN 17

#define MICROPHONES_I2S_NUM I2S_NUM_0
#define MICROPHONES_WS_PIN 32
#define MICROPHONES_BCK_PIN 27
#define MICROPHONES_DATA_PIN 33

#define BONE_CONDUCTORS_I2S_NUM I2S_NUM_1
#define BONE_CONDUCTORS_WS_PIN 3
#define BONE_CONDUCTORS_BCK_PIN 21
#define BONE_CONDUCTORS_DATA_PIN 19

#define DMA_BUFFER_COUNT 8
#define DMA_BUFFER_LEN 1024

void i2s_setup();

void speakers_setup();
void microphones_setup();
void bone_conductors_setup();
