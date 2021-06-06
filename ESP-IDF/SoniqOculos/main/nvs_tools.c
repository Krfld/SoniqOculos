#include "nvs_tools.h"

static nvs_handle_t s_nvs_handle;

void nvs_init()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}
/*void nvs_deinit()
{
    nvs_flash_deinit();
}*/

void nvs_write(int32_t value, char *namespace)
{
    if (nvs_open(namespace, NVS_READWRITE, &s_nvs_handle) != ESP_OK)
    {
        ESP_LOGE(NVS_TAG, "NVS open failed");
        return;
    }

    if (nvs_set_u32(s_nvs_handle, namespace, value) != ESP_OK) // Set value
        ESP_LOGE(NVS_TAG, "NVS set failed");
    if (nvs_commit(s_nvs_handle) != ESP_OK) // Commit value
        ESP_LOGE(NVS_TAG, "NVS commit failed");

    nvs_close(s_nvs_handle);
}

int32_t nvs_read(char *namespace)
{
    if (nvs_open(namespace, NVS_READWRITE, &s_nvs_handle) != ESP_OK)
    {
        ESP_LOGE(NVS_TAG, "NVS open failed");
        return DEFAULT_READ_VALUE;
    }

    uint32_t value = DEFAULT_READ_VALUE;
    esp_err_t err = nvs_get_u32(s_nvs_handle, namespace, &value); // Get value
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
        ESP_LOGE(NVS_TAG, "NVS get failed");

    nvs_close(s_nvs_handle);

    return value;
}
