#ifndef FLASH_STORAGE
#define FLASH_STORAGE

#include "esp_err.h"

#define SSID_MAX_LEN            33 //Includes null terminator
#define PASS_MAX_LEN            65 //Includes null terminator

void initFlashStorage();

esp_err_t storeWifiCredentials(char *ssid, char *password);

esp_err_t loadWifiCredentials(char *ssid, char *password, uint8_t* ssidLen, uint8_t* passLen);

#endif
