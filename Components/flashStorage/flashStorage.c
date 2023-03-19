#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "flashStorage.h"

#define TAG                     "Flash_Storage"
#define SETTINGS_NVS_NAMESPACE  "userSettings"

void initFlashStorage()
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_LOGW(TAG, "Failed to initialize NVS, retrying..");
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
        
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
        {
            ESP_LOGE(TAG, "Retry failed to initialize NVS");
        }
    ESP_ERROR_CHECK( err );
    }
}

esp_err_t storeWifiCredentials(char *ssid, char *password)
{
    esp_err_t err;
    nvs_handle_t nvsHandle;

    // Open NVS namespace
    err = nvs_open(SETTINGS_NVS_NAMESPACE, NVS_READWRITE, &nvsHandle);
    if (err != ESP_OK) {
        return err;
    }

    // Write SSID and password to NVS
    err = nvs_set_str(nvsHandle, "WIFI_ssid", ssid);
    if (err != ESP_OK) {
        nvs_close(nvsHandle);
        return err;
    }
    err = nvs_set_str(nvsHandle, "WIFI_pass", password);
    if (err != ESP_OK) {
        nvs_close(nvsHandle);
        return err;
    }

    // Commit the data to flash memory
    err = nvs_commit(nvsHandle);
    if (err != ESP_OK) {
        nvs_close(nvsHandle);
        return err;
    }

    // Close the NVS namespace handle
    nvs_close(nvsHandle);
    return ESP_OK;
}

//TODO: get the size of the value and allocate the required space and return that pointer
// https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-reference/storage/nvs_flash.html#_CPPv411nvs_get_str12nvs_handle_tPKcPcP6size_t
esp_err_t load_wifi_credentials(char *ssid, char *password, size_t max_len)
{
    esp_err_t err;
    nvs_handle_t nvs_handle;

    // Open NVS namespace 
    err = nvs_open(SETTINGS_NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        return err;
    }

    // Read SSID and password from NVS
    size_t len = max_len;
    err = nvs_get_str(nvs_handle, "WIFI_ssid", ssid, &len);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return err;
    }
    len = max_len;
    err = nvs_get_str(nvs_handle, "WIFI_pass", password, &len);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return err;
    }

    // Close the NVS namespace handle
    nvs_close(nvs_handle);
    return ESP_OK;
}