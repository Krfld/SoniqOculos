#include "main.h"
#include "tools.h"

#include "bt_app_core.h"
#include "bt_app_av.h"

#define BT_SPP_TAG "BT_SPP"

// event for handler "bt_av_hdl_stack_up
enum
{
    BT_APP_EVT_STACK_UP = 0,
};

// handler for bluetooth stack enabled events
void bt_av_hdl_stack_evt(uint16_t event, void *p_param);

void bt_init();
