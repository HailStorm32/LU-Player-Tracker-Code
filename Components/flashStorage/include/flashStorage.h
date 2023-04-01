#ifndef FLASH_STORAGE
#define FLASH_STORAGE

#include "esp_err.h"

void initFlashStorage();

esp_err_t storeWifiCredentials(char *ssid, char *password);

esp_err_t loadWifiCredentials(char *ssid, char *password, uint8_t* ssidLen, uint8_t* passLen);

#endif
