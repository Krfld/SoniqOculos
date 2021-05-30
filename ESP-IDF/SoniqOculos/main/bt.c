// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "bt.h"

static uint8_t *bda = NULL; //! Deep-sleep
void set_bda(uint8_t *addr)
{
    if (bda == NULL)
        bda = (uint8_t *)malloc(ESP_BD_ADDR_LEN);
    memcpy(bda, addr, ESP_BD_ADDR_LEN);
}

static bool bt_music_ready = false;

// event for handler "bt_av_hdl_stack_up
enum
{
    BT_APP_EVT_STACK_UP = 0,
};

// handler for bluetooth stack enabled events
void bt_av_hdl_stack_evt(uint16_t event, void *p_param);

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    ////ESP_LOGI(BT_SPP_TAG, "SPP event: %d", event);
    switch (event)
    {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(BT_SPP_TAG, "ESP_SPP_INIT_EVT");
        esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, DEVICE_NAME);
        break;
    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(BT_SPP_TAG, "ESP_SPP_CLOSE_EVT");
        ESP_LOGW(BT_SPP_TAG, "Disconnected from server");
        break;
    case ESP_SPP_DATA_IND_EVT:
        ESP_LOGI(BT_SPP_TAG, "ESP_SPP_DATA_IND_EVT len=%d handle=%d",
                 param->data_ind.len, param->data_ind.handle);
        if (param->data_ind.len < MSG_BUFFER_SIZE)
        {
            char msg[MSG_BUFFER_SIZE];
            snprintf(msg, param->data_ind.len + 1, (char *)param->data_ind.data); // Filter received message contents
            handleMsgs(msg);                                                      // Handle message received and response
            esp_spp_write(param->write.handle, strlen(msg), (uint8_t *)msg);      // Send response
        }
        else
        {
            esp_log_buffer_hex("", param->data_ind.data, param->data_ind.len);
        }
        break;
    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(BT_SPP_TAG, "ESP_SPP_SRV_OPEN_EVT");
        ESP_LOGW(BT_SPP_TAG, "Connected to server");
        break;
    default:
        break;
    }
}

void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BT_GAP_AUTH_CMPL_EVT:
    {
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(BT_AV_TAG, "authentication success: %s", param->auth_cmpl.device_name);
            esp_log_buffer_hex(BT_AV_TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
        }
        else
        {
            ESP_LOGE(BT_AV_TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
        break;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    case ESP_BT_GAP_CFM_REQ_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d", param->key_notif.passkey);
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
        break;
#endif

    default:
    {
        ESP_LOGI(BT_AV_TAG, "event: %d", event);
        break;
    }
    }
    return;
}

void bt_av_hdl_stack_evt(uint16_t event, void *p_param)
{
    ESP_LOGD(BT_AV_TAG, "%s evt %d", __func__, event);
    switch (event)
    {
    case BT_APP_EVT_STACK_UP:
    {
        // set up device name
        esp_bt_dev_set_device_name(DEVICE_NAME);

        esp_bt_gap_register_callback(bt_app_gap_cb);

        // set discoverable and connectable mode, wait to be connected
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        break;
    }
    default:
        ESP_LOGE(BT_AV_TAG, "%s unhandled evt %d", __func__, event);
        break;
    }
}

void bt_init()
{
    // Initialize NVS — it is used to store PHY calibration data
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((err = esp_bt_controller_init(&bt_cfg)) != ESP_OK)
    {
        ESP_LOGE(BT_AV_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK)
    {
        ESP_LOGE(BT_AV_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_init()) != ESP_OK)
    {
        ESP_LOGE(BT_AV_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_enable()) != ESP_OK)
    {
        ESP_LOGE(BT_AV_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_spp_register_callback(esp_spp_cb)) != ESP_OK)
    {
        ESP_LOGE(BT_AV_TAG, "%s spp register failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_spp_init(ESP_SPP_MODE_CB)) != ESP_OK)
    {
        ESP_LOGE(BT_AV_TAG, "%s spp init failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    // create application task
    bt_app_task_start_up();

    // Bluetooth device name, connection mode and profile set up
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);

#if (CONFIG_BT_SSP_ENABLED == true)
    // Set default parameters for Secure Simple Pairing
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
#endif

    // Set default parameters for Legacy Pairing Use fixed pin code
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
    esp_bt_pin_code_t pin_code;
    pin_code[0] = '1';
    pin_code[1] = '2';
    pin_code[2] = '3';
    pin_code[3] = '4';
    esp_bt_gap_set_pin(pin_type, 4, pin_code);
}

void bt_music_init()
{
    if (bt_music_ready)
        return;

    // initialize AVRCP controller
    esp_avrc_ct_init();
    esp_avrc_ct_register_callback(bt_app_rc_ct_cb);
    // initialize AVRCP target
    esp_avrc_tg_init();
    esp_avrc_tg_register_callback(bt_app_rc_tg_cb);

    /*esp_avrc_rn_evt_cap_mask_t evt_set = {0};
    esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_SET, &evt_set, ESP_AVRC_RN_VOLUME_CHANGE);
    assert(esp_avrc_tg_set_rn_evt_cap(&evt_set) == ESP_OK);*/

    // initialize A2DP sink
    esp_a2d_register_callback(&bt_app_a2d_cb);
    esp_a2d_sink_register_data_callback(bt_app_a2d_data_cb);
    esp_a2d_sink_init();

    if (bda != NULL)
        esp_a2d_sink_connect(bda); // Connect to previous device

    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    bt_music_ready = true;
}
void bt_music_deinit()
{
    if (!bt_music_ready)
        return;

    if (bda != NULL)
        esp_a2d_sink_disconnect(bda); // Disconnect from device

    esp_avrc_ct_deinit();
    esp_avrc_tg_deinit();
    esp_a2d_sink_deinit();

    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    bt_music_ready = false;
}

void bt_send_avrc_cmd(uint8_t cmd)
{
    static uint8_t tl = 0; // 'static' will keep value

    if (++tl > 15) // "consecutive commands should use different values"
        tl = 0;

    esp_avrc_ct_send_passthrough_cmd(tl, cmd, ESP_AVRC_PT_CMD_STATE_PRESSED); // Send AVRCP command
}