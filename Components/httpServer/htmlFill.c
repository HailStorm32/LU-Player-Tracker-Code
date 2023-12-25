#include <ctype.h>
#include <string.h>

#include "htmlFill.h"
#include "flashStorage.h"

void fillWifiHtml(char *htmlCode)
{
    uint32_t length = strlen(htmlCode);

    // Check to see if there is a stored SSID and password
    char ssid[SSID_MAX_LEN];
    char password[PASS_MAX_LEN];
    uint8_t ssidLen = 0;
    uint8_t passLen = 0;

    esp_err_t err = loadWifiCredentials(ssid, password, &ssidLen, &passLen);

    // If there is a stored SSID and password, fill the HTML code with it





}