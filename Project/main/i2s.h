#include "main.h"

#define BONE_CONDUCTORS I2S_NUM_0
#define SPEAKERS I2S_NUM_1

#define I2S_WS_PIN(i2s_num) (!i2s_num ? 32 : 5)
#define I2S_BCK_PIN(i2s_num) (!i2s_num ? 33 : 17)
#define I2S_DATA_PIN(i2s_num) (!i2s_num ? 27 : 16)

#define DMA_BUFFER_COUNT 8
#define DMA_BUFFER_LEN 1024

void i2s_setup();