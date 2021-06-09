#include "bt.h"

static void bt_app_task_handler(void *arg);
static bool bt_app_send_msg(bt_app_msg_t *msg);
static void bt_app_work_dispatched(bt_app_msg_t *msg);

static xQueueHandle s_bt_app_task_queue = NULL;
static xTaskHandle s_bt_app_task_handle = NULL;
static xTaskHandle s_bt_i2s_task_handle = NULL;
static RingbufHandle_t s_ringbuf_i2s = NULL;
static QueueHandle_t bt_i2s_queue_handle = NULL;

bool bt_app_work_dispatch(bt_app_cb_t p_cback, uint16_t event, void *p_params, int param_len, bt_app_copy_cb_t p_copy_cback)
{
    ESP_LOGD(BT_APP_CORE_TAG, "%s event 0x%x, param len %d", __func__, event, param_len);

    bt_app_msg_t msg;
    memset(&msg, 0, sizeof(bt_app_msg_t));

    msg.sig = BT_APP_SIG_WORK_DISPATCH;
    msg.event = event;
    msg.cb = p_cback;

    if (param_len == 0)
    {
        return bt_app_send_msg(&msg);
    }
    else if (p_params && param_len > 0)
    {
        if ((msg.param = malloc(param_len)) != NULL)
        {
            memcpy(msg.param, p_params, param_len);
            /* check if caller has provided a copy callback to do the deep copy */
            if (p_copy_cback)
            {
                p_copy_cback(&msg, msg.param, p_params);
            }
            return bt_app_send_msg(&msg);
        }
    }

    return false;
}

static bool bt_app_send_msg(bt_app_msg_t *msg)
{
    if (msg == NULL)
    {
        return false;
    }

    if (xQueueSend(s_bt_app_task_queue, msg, 10 / portTICK_RATE_MS) != pdTRUE)
    {
        ESP_LOGE(BT_APP_CORE_TAG, "%s xQueue send failed", __func__);
        return false;
    }
    return true;
}

static void bt_app_work_dispatched(bt_app_msg_t *msg)
{
    if (msg->cb)
    {
        msg->cb(msg->event, msg->param);
    }
}

static void bt_app_task_handler(void *arg)
{
    bt_app_msg_t msg;
    for (;;)
    {
        if (pdTRUE == xQueueReceive(s_bt_app_task_queue, &msg, (portTickType)portMAX_DELAY))
        {
            ESP_LOGD(BT_APP_CORE_TAG, "%s, sig 0x%x, 0x%x", __func__, msg.sig, msg.event);
            switch (msg.sig)
            {
            case BT_APP_SIG_WORK_DISPATCH:
                bt_app_work_dispatched(&msg);
                break;
            default:
                ESP_LOGW(BT_APP_CORE_TAG, "%s, unhandled sig: %d", __func__, msg.sig);
                break;
            } // switch (msg.sig)

            if (msg.param)
            {
                free(msg.param);
            }
        }
    }
}

void bt_app_task_start_up(void)
{
    s_bt_app_task_queue = xQueueCreate(10, sizeof(bt_app_msg_t));
    xTaskCreate(bt_app_task_handler, "BtAppT", 3072, NULL, configMAX_PRIORITIES - 3, &s_bt_app_task_handle);
    return;
}

void bt_app_task_shut_down(void)
{
    if (s_bt_app_task_handle)
    {
        vTaskDelete(s_bt_app_task_handle);
        s_bt_app_task_handle = NULL;
    }
    if (s_bt_app_task_queue)
    {
        vQueueDelete(s_bt_app_task_queue);
        s_bt_app_task_queue = NULL;
    }
}

static void bt_i2s_task_handler(void *arg)
{
    uint8_t *data = NULL;
    size_t size = 0;

    for (;;)
    {
        //TODO Fix | Not sending avrcp commands

        if (!xQueueReceive(bt_i2s_queue_handle, &size, portMAX_DELAY)) // Wait for ringbuffer to have at least 4096 bytes
            continue;

        data = (uint8_t *)xRingbufferReceiveUpTo(s_ringbuf_i2s, &size, (portTickType)portMAX_DELAY, DATA_LENGTH); // Get 4096 bytes

        if (size != DATA_LENGTH)
        {
            ESP_LOGE(BT_APP_CORE_TAG, "Packet size different than %d: %d", DATA_LENGTH, size);
            ////vRingbufferReturnItem(s_ringbuf_i2s, data);
            ////continue;
        }

        process_data(data, &size);

        vRingbufferReturnItem(s_ringbuf_i2s, data);
    }
}

void bt_i2s_task_start_up(void)
{
    bt_i2s_queue_handle = xQueueCreate(1, sizeof(bool));
    if (!bt_i2s_queue_handle)
        ESP_LOGE(BT_APP_CORE_TAG, "Error creating queue");

    s_ringbuf_i2s = xRingbufferCreate(RINGBUFFER_SIZE, RINGBUF_TYPE_BYTEBUF);
    if (!s_ringbuf_i2s)
        ESP_LOGE(BT_APP_CORE_TAG, "Error creating ringbuffer");

    if (!xTaskCreate(bt_i2s_task_handler, "BtI2ST", BT_I2S_STACK_DEPTH, NULL, configMAX_PRIORITIES, &s_bt_i2s_task_handle))
        ESP_LOGE(BT_APP_CORE_TAG, "Error creating BtI2ST task");
}

void bt_i2s_task_shut_down(void)
{
    if (s_bt_i2s_task_handle)
    {
        vTaskDelete(s_bt_i2s_task_handle);
        s_bt_i2s_task_handle = NULL;
    }

    delay(100); // Make sure task is deleted before deleting ringbuffer

    if (s_ringbuf_i2s)
    {
        vRingbufferDelete(s_ringbuf_i2s);
        s_ringbuf_i2s = NULL;
    }
    if (bt_i2s_queue_handle)
    {
        vQueueDelete(bt_i2s_queue_handle);
        bt_i2s_queue_handle = NULL;
    }
}

size_t write_ringbuf(const uint8_t *data, size_t size)
{
    if (!s_ringbuf_i2s) // Check if buffer is created
        return 0;

    BaseType_t done = xRingbufferSend(s_ringbuf_i2s, (void *)data, size, (portTickType)portMAX_DELAY);

    if (RINGBUFFER_SIZE - xRingbufferGetCurFreeSize(s_ringbuf_i2s) > DATA_LENGTH) // Checks if ringbuffer has at least 4096 bytes
        xQueueSend(bt_i2s_queue_handle, &size, portMAX_DELAY);                    //TODO Test

    if (done)
        return size;
    return 0;
}

void bt_send_avrc_cmd(uint8_t cmd)
{
    static uint8_t tl = 0; // 'static' will keep value

    if (++tl > 15) // "consecutive commands should use different values"
        tl = 0;

    esp_avrc_ct_send_passthrough_cmd(tl, cmd, ESP_AVRC_PT_CMD_STATE_PRESSED);  // Send AVRCP command pressing
    esp_avrc_ct_send_passthrough_cmd(tl, cmd, ESP_AVRC_PT_CMD_STATE_RELEASED); // Send AVRCP command releasing
}
