#include "main.h"

#define SPP_SERVER_NAME "SPP_SoniqOculos"

/* event for handler "bt_av_hdl_stack_up */
enum
{
    BT_APP_EVT_STACK_UP = 0,
};

//void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
void bt_init();