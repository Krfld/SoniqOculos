#include "main.h"

#define I2S_WS_PIN(i2s_num) (!i2s_num ? 13 : 27)
#define I2S_BCK_PIN(i2s_num) (!i2s_num ? 12 : 33)
#define I2S_DATA_PIN(i2s_num) (!i2s_num ? 14 : 32)

void i2s_setup();