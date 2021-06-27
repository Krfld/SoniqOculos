#ifndef __BT_APP_AV_H__
#define __BT_APP_AV_H__

#include "main.h"

bool bt_is_connected();
bool bt_is_music_playing();

void bt_reset();

/**
 * @brief     callback function for A2DP sink
 */
void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);

/**
 * @brief     callback function for A2DP sink audio data stream
 */
void bt_app_a2d_data_cb(const uint8_t *data, uint32_t len);

/**
 * @brief     callback function for AVRCP controller
 */
void bt_app_rc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param);

/**
 * @brief     callback function for AVRCP target
 */
void bt_app_rc_tg_cb(esp_avrc_tg_cb_event_t event, esp_avrc_tg_cb_param_t *param);

#endif /* __BT_APP_AV_H__*/
