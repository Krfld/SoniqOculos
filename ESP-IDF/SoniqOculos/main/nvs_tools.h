#include "main.h"

#define DEFAULT_READ_VALUE 1

/**
 * @brief Prepare to write to flash
 * 
 */
void nvs_init();

/**
 * @brief Write int value to flash
 * 
 * @param value int to write
 * @param namespace ID of the value
 */
void nvs_write(int32_t value, char *namespace);
/**
 * @brief Read int value from flash
 * 
 * @param namespace ID of the value
 * @return int32_t int read
 */
int32_t nvs_read(char *namespace);