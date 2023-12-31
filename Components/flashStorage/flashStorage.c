#include "esp_log.h"
#include "nvs_flash.h"
#include "flashStorage.h"

#include <string.h>

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

    // //Clear the namespace DEBUG ONLY
    // nvs_handle_t nvsHandle;
    // err = nvs_open(SETTINGS_NVS_NAMESPACE, NVS_READWRITE, &nvsHandle);
    // if (err != ESP_OK) {
    //     ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    // }
    // else
    // {
    //     ESP_LOGI(TAG, "Clearing NVS namespace");
    //     nvs_erase_all(nvsHandle);
    //     nvs_commit(nvsHandle);
    //     nvs_close(nvsHandle);
    // }

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
    bool allSettingsFound = true;
    
    //NOTE: The function will return whatever values it was able to find, even if it was not able to find all of them
    // Data validation should be done by the caller

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
            ESP_LOGE(TAG, "Given SSID buffer is NULL");
            nvs_close(nvsHandle);
            return ESP_FAIL;
        }

        //Get SSID
        nvs_get_str(nvsHandle, "WIFI_ssid", ssid, &strLen);
    }
    else if(err == ESP_ERR_NVS_NOT_FOUND)   
    {
        ESP_LOGI(TAG, "SSID in NVM not found");
        allSettingsFound = false;
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

        if (password == NULL)
        {
            ESP_LOGE(TAG, "Given password buffer is NULL");
            nvs_close(nvsHandle);
            return ESP_FAIL;
        }

        //Get password
        nvs_get_str(nvsHandle, "WIFI_pass", password, &strLen);
    }
    else if(err == ESP_ERR_NVS_NOT_FOUND)   
    {
        ESP_LOGI(TAG, "Password in NVM not found");
        allSettingsFound = false;
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
    
    if(!allSettingsFound)
    {
        return ESP_ERR_NVS_NOT_FOUND;
    }
    else
    {
        return ESP_OK;
    }
}

esp_err_t storeMqttSettings(mqttSettings_t *mqttSettings)
{
    esp_err_t err;
    nvs_handle_t nvsHandle;

    if(mqttSettings == NULL) {
        ESP_LOGE(TAG, "Given mqttSettings is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Open NVS namespace
    err = nvs_open(SETTINGS_NVS_NAMESPACE, NVS_READWRITE, &nvsHandle);
    if (err != ESP_OK) {
        return err;
    }

    // Write MQTT settings to NVS
    if (strlen(mqttSettings->address) != 0)
    {
        // Write MQTT address to NVS
        err = nvs_set_str(nvsHandle, "MQTT_addr", mqttSettings->address);
        if (err != ESP_OK) {
            nvs_close(nvsHandle);
            return err;
        }
    }

    if (strlen(mqttSettings->username) != 0)
    {
        // Write MQTT username to NVS
        err = nvs_set_str(nvsHandle, "MQTT_uname", mqttSettings->username);
        if (err != ESP_OK) {
            nvs_close(nvsHandle);
            return err;
        }
    }

    if (strlen(mqttSettings->password) != 0)
    {
        // Write MQTT password to NVS
        err = nvs_set_str(nvsHandle, "MQTT_pass", mqttSettings->password);
        if (err != ESP_OK) {
            nvs_close(nvsHandle);
            return err;
        }
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

esp_err_t loadMqttSettings(mqttSettings_t *mqttSettings)
{
    esp_err_t err;
    nvs_handle_t nvsHandle;
    size_t strLen = NULL;
    bool allSettingsFound = true;

    //NOTE: The function will return whatever values it was able to find, even if it was not able to find all of them
    // Data validation should be done by the caller

    if(mqttSettings == NULL) {
        ESP_LOGE(TAG, "Given mqttSettings is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Clear the mqttSettings struct
    memset(mqttSettings, 0, sizeof(mqttSettings_t));

    // Open NVS namespace 
    err = nvs_open(SETTINGS_NVS_NAMESPACE, NVS_READONLY, &nvsHandle);
    if (err != ESP_OK) {
        return err;
    }

    // Get MQTT settings from from NVS

    // Get broker address size, then get broker address value
    if (( err = nvs_get_str(nvsHandle, "MQTT_addr", NULL, &strLen)) == ESP_OK)
    {
        mqttSettings->addressLen = strLen;

        // Get address
        nvs_get_str(nvsHandle, "MQTT_addr", mqttSettings->address, &strLen);
    }
    else if(err == ESP_ERR_NVS_NOT_FOUND)   
    {
        ESP_LOGI(TAG, "MQTT address in NVM not found");
        allSettingsFound = false;
    }
    else   
    {
        nvs_close(nvsHandle);
        ESP_LOGE(TAG, "Failed to retrieve mqtt address from NVM");
        //TODO: Handle and report the different error types
        // https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-reference/storage/nvs_flash.html#_CPPv411nvs_get_str12nvs_handle_tPKcPcP6size_t:~:text=includes%20zero%20terminator.-,Returns,-ESP_OK%20if%20the
        return err;
    }

    // Get username size, then get username value
    if ((err = nvs_get_str(nvsHandle, "MQTT_uname", NULL, &strLen)) == ESP_OK)
    {
        mqttSettings->usernameLen = strLen;

        // Get username
        nvs_get_str(nvsHandle, "MQTT_uname", mqttSettings->username, &strLen);
    }
    else if(err == ESP_ERR_NVS_NOT_FOUND)   
    {
        ESP_LOGI(TAG, "MQTT username in NVM not found");
        allSettingsFound = false;
    }
    else   
    {
        nvs_close(nvsHandle);
        ESP_LOGE(TAG, "Failed to retrieve mqtt username from NVM");
        //TODO: Handle and report the different error types
        // https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-reference/storage/nvs_flash.html#_CPPv411nvs_get_str12nvs_handle_tPKcPcP6size_t:~:text=includes%20zero%20terminator.-,Returns,-ESP_OK%20if%20the
    }

    // Get password size, then get password value   
    if ((err = nvs_get_str(nvsHandle, "MQTT_pass", NULL, &strLen)) == ESP_OK)
    {
        mqttSettings->passwordLen = strLen;

        // Get password
        nvs_get_str(nvsHandle, "MQTT_pass", mqttSettings->password, &strLen);
    }
    else if(err == ESP_ERR_NVS_NOT_FOUND)   
    {
        ESP_LOGI(TAG, "MQTT password in NVM not found");
        allSettingsFound = false;
    }
    else   
    {
        nvs_close(nvsHandle);
        ESP_LOGE(TAG, "Failed to retrieve mqtt password from NVM");
        //TODO: Handle and report the different error types
        // https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-reference/storage/nvs_flash.html#_CPPv411nvs_get_str12nvs_handle_tPKcPcP6size_t:~:text=includes%20zero%20terminator.-,Returns,-ESP_OK%20if%20the
    }

    // Close the NVS namespace handle
    nvs_close(nvsHandle);

    if(!allSettingsFound)
    {
        return ESP_ERR_NVS_NOT_FOUND;
    }
    else
    {
        return ESP_OK;
    }
}