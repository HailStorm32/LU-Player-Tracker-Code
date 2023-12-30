#ifndef FLASH_STORAGE
#define FLASH_STORAGE

#include "esp_err.h"

#define WIFI_SSID_MAX_LEN           33 //Includes null terminator
#define WIFI_PASS_MAX_LEN           65 //Includes null terminator
#define MQTT_ADDR_MAX_LEN           65 //Includes null terminator
#define MQTT_UNAME_MAX_LEN          65 //Includes null terminator
#define MQTT_PASS_MAX_LEN           65 //Includes null terminator

typedef struct
{
    char address[MQTT_ADDR_MAX_LEN];
    uint8_t addressLen;
    
    char username[MQTT_UNAME_MAX_LEN];
    uint8_t usernameLen;

    char password[MQTT_PASS_MAX_LEN];
    uint8_t passwordLen;
} mqttSettings_t;

void initFlashStorage();

esp_err_t storeWifiCredentials(char *ssid, char *password);

esp_err_t loadWifiCredentials(char *ssid, char *password, uint8_t* ssidLen, uint8_t* passLen);

esp_err_t storeMqttSettings(mqttSettings_t *mqttSettings);

esp_err_t loadMqttSettings(mqttSettings_t *mqttSettings);

#endif
 