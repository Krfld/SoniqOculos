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

    if (nvs_open("storage", NVS_READWRITE, &s_nvs_handle) != ESP_OK)
        ESP_LOGE(NVS_TAG, "nvs open failed");
}
/*void nvs_deinit()
{
    nvs_close(s_nvs_handle);
    s_nvs_handle = NULL;
    nvs_flash_deinit();
}*/

void nvs_write(int32_t value)
{
    if (nvs_set_u32(s_nvs_handle, "value", value) != ESP_OK) // Set value
        ESP_LOGE(NVS_TAG, "nvs set failed");
    if (nvs_commit(s_nvs_handle) != ESP_OK) // Commit value
        ESP_LOGE(NVS_TAG, "nvs commit failed");
}

int32_t nvs_read()
{
    uint32_t value;
    esp_err_t err = nvs_get_u32(s_nvs_handle, "value", &value); // Get value
    if (err != ESP_OK)
    {
        ESP_LOGE(NVS_TAG, "nvs get failed");
        return 1; // Default value
    }
    return value;
}
