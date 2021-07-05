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
 * @param namespace ID of the value
 * @param value int to write
 */
void nvs_write(char *namespace, int32_t value);
/**
 * @brief Read int value from flash
 * 
 * @param namespace ID of the value
 * @return int32_t int read
 */
int32_t nvs_read(char *namespace);