/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/****************************************************************************
* The demo shows BLE and classic Bluetooth coexistence. You can use BLE GATT server and classic bluetooth A2DP together.
* The BLE GATT server part of the demo creates a GATT service and then starts advertising, waiting to be connected by a GATT client.
* After the program is started, a GATT client can discover the device named "ESP_COEX_BLE_DEMO". Once the connection is established,
* GATT client can read or write data to the device. It can also receive notification or indication data.
* Attention: If you test the demo with iPhone, BLE GATT server adv name will change to "ESP_COEX_A2DP_DEMO" after you connect it.
* The classic bluetooth A2DP part of the demo implements Advanced Audio Distribution Profile to receive an audio stream.
* After the program is started, other bluetooth devices such as smart phones can discover the device named "ESP_COEX_A2DP_DEMO".
* Once the connection is established, audio data can be transmitted. This will be visible in the application log including a count
* of audio data packets.
****************************************************************************/

#include "main.h"

#include "bt.h"
#include "ble.h"

void app_main(void)
{

    i2s_setup();

    bt_init();
    ble_gatts_init();

    gpio_pad_select_gpio(LED_BUILTIN);
    gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);
    //gpio_set_level(LED_BUILTIN, LOW);

    /// Right
    dac_output_enable(DAC_CHANNEL_1);
    dac_output_voltage(DAC_CHANNEL_1, 1.2 / VDD * 255);

    /// Left
    dac_output_enable(DAC_CHANNEL_2);
    dac_output_voltage(DAC_CHANNEL_2, 1.6 / VDD * 255);

    delay(1000);

    printf("READY\n");
    gpio_set_level(LED_BUILTIN, HIGH);
}

void delay(int millis)
{
    vTaskDelay(millis / portTICK_RATE_MS);
}

void process_data(const uint8_t *data, size_t len)
{
    size_t i2s0_bytes_written = 0, i2s1_bytes_written = 0;

    /*for (size_t i = 0; i < len; i += 4)
    {
        printf("Left %c ", (signed char)data[i]);
        printf("%c\n", (signed char)data[i + 1]);

        printf("Right %c ", (signed char)data[i + 2]);
        printf("%c\n", (signed char)data[i + 3]);
    }*/

    //TODO Write to i2s
    i2s_write(I2S_NUM_0, data, len, &i2s0_bytes_written, portMAX_DELAY);
    i2s_write(I2S_NUM_1, data, len, &i2s1_bytes_written, portMAX_DELAY);
}

void i2s_setup()
{
    i2s_config_t i2s_output_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = 2,
        .dma_buf_len = 1024,
        .intr_alloc_flags = 0, // default interrupt priority
        .tx_desc_auto_clear = true,
        .use_apll = false};

    /// I2S0
    if (i2s_driver_install(I2S_NUM_0, &i2s_output_config, 0, NULL) != ESP_OK)
        printf("I2S0 Driver install failed\n");

    i2s_pin_config_t i2s0_pin_config = {
        .ws_io_num = I2S_WS_PIN(I2S_NUM_0),
        .bck_io_num = I2S_BCK_PIN(I2S_NUM_0),
        .data_out_num = I2S_DATA_PIN(I2S_NUM_0),
        .data_in_num = I2S_PIN_NO_CHANGE};
    if (i2s_set_pin(I2S_NUM_0, &i2s0_pin_config) != ESP_OK)
        printf("I2S0 Set pin failed\n");

    /// I2S1
    if (i2s_driver_install(I2S_NUM_1, &i2s_output_config, 0, NULL) != ESP_OK)
        printf("I2S1 Driver install failed\n");

    i2s_pin_config_t i2s1_pin_config = {
        .ws_io_num = I2S_WS_PIN(I2S_NUM_1),
        .bck_io_num = I2S_BCK_PIN(I2S_NUM_1),
        .data_out_num = I2S_DATA_PIN(I2S_NUM_1),
        .data_in_num = I2S_PIN_NO_CHANGE};
    if (i2s_set_pin(I2S_NUM_1, &i2s1_pin_config) != ESP_OK)
        printf("I2S1 Set pin failed\n");
}