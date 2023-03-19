#ifndef FLASH_STORAGE
#define FLASH_STORAGE

#include "esp_err.h"

void initFlashStorage();

esp_err_t store_wifi_credentials(char *ssid, char *password);

esp_err_t load_wifi_credentials(char *ssid, char *password, size_t max_len);

#endif
