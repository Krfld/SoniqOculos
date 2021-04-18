#include "main.h"

#define GATTS_SERVICE_UUID_A 0x00FF
#define GATTS_CHAR_UUID_A 0xFF01
#define GATTS_DESCR_UUID_A 0x3333
#define GATTS_NUM_HANDLE_A 4

#define GATTS_SERVICE_UUID_B 0x00EE
#define GATTS_CHAR_UUID_B 0xEE01
#define GATTS_DESCR_UUID_B 0x2222
#define GATTS_NUM_HANDLE_B 4

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40
#define PREPARE_BUF_MAX_SIZE 1024
#define PROFILE_NUM 2
#define PROFILE_A_APP_ID 0
#define PROFILE_B_APP_ID 1

typedef struct
{
    uint8_t *prepare_buf;
    int prepare_len;
} prepare_type_env_t;

void ble_gatts_init();