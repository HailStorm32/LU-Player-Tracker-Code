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

esp_err_t loadWifiCredentials(char *ssid, char *password, uint8_t* ssidLen, uint8_t* passLen)
{
    esp_err_t err;
    nvs_handle_t nvsHandle;
    size_t strLen = NULL;
    

    // Open NVS namespace 
    err = nvs_open(SETTINGS_NVS_NAMESPACE, NVS_READONLY, &nvsHandle);
    if (err != ESP_OK) {
        return err;
    }

    // Read SSID and password from NVS

    //Get SSID size
    if (nvs_get_str(nvsHandle, "WIFI_ssid", NULL, &strLen) == ESP_OK)
    {
        *ssidLen = strLen;

        if (ssid == NULL)
        {
            ESP_LOGE(TAG, "Unable to allocate space for ssid");
            nvs_close(nvsHandle);
            return ESP_FAIL;
        }

        //Get SSID
        nvs_get_str(nvsHandle, "WIFI_ssid", ssid, &strLen);
    }
    else   
    {
        nvs_close(nvsHandle);
        ESP_LOGE(TAG, "Failed to retrieve password");
        //TODO: Handle and report the different error types
        // https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-reference/storage/nvs_flash.html#_CPPv411nvs_get_str12nvs_handle_tPKcPcP6size_t:~:text=includes%20zero%20terminator.-,Returns,-ESP_OK%20if%20the
        return err;
    }


    //Get password size
    if (nvs_get_str(nvsHandle, "WIFI_pass", NULL, &strLen) == ESP_OK)
    {
        *passLen = strLen;

        if (ssid == NULL)
        {
            ESP_LOGE(TAG, "Unable to allocate space for password");
            nvs_close(nvsHandle);
            return ESP_FAIL;
        }

        //Get SSID
        nvs_get_str(nvsHandle, "WIFI_pass", password, &strLen);
    }
    else   
    {
        nvs_close(nvsHandle);
        ESP_LOGE(TAG, "Failed to retrieve password");
        //TODO: Handle and report the different error types
        // https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-reference/storage/nvs_flash.html#_CPPv411nvs_get_str12nvs_handle_tPKcPcP6size_t:~:text=includes%20zero%20terminator.-,Returns,-ESP_OK%20if%20the
        return err;
    }

    // Close the NVS namespace handle
    nvs_close(nvsHandle);
    return ESP_OK;
}