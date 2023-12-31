#include <ctype.h>
#include <string.h>

#include "htmlFill.h"
#include "flashStorage.h"


#define TEMPLATE_KEY_LEN    5

#define WIFI_NUM_KEYS       1
#define MQTT_NUM_KEYS       0
#define LED_NUM_KEYS        0

char * searchAndReplace(char *str, char *search, char *replace);

char * searchAndReplace(char *str, char *search, char *replace)
{
    char *pos, *temp;
    int searchLen = strlen(search);
    int replaceLen = strlen(replace);

    // Search for the key in the string
    pos = strstr(str, search);

    // If the key is found, replace it
    if (pos != NULL)
    {
        // Allocate a temporary string to hold the new string
        temp = malloc(strlen(str) - searchLen + replaceLen + 1);

        // Copy the string up to the key
        strncpy(temp, str, pos - str);

        // Copy the replacement string
        strcpy(temp + (pos - str), replace);

        // Copy the rest of the string
        strcpy(temp + (pos - str) + replaceLen, pos + searchLen);

        // Free the old string
        free(str);

        // Return the new string
        return temp;
    }

    // If the key is not found, return the original string
    return str;
}

void fillWifiHtmlTmpl(char **htmlCodeBuffer)
{
    char ssid[WIFI_SSID_MAX_LEN];
    char password[WIFI_PASS_MAX_LEN];
    uint8_t ssidLen = 0;
    uint8_t passLen = 0;

    esp_err_t err = loadWifiCredentials(ssid, password, &ssidLen, &passLen);

    if (err != ESP_OK && err != ESP_ERR_NOT_FOUND) // We dont care if the settings are not found, we will just use the defaults
    {
        return;
    }
    
    //Fill in and replace the keys in the HTML template

    if (ssidLen != 0)
    {   
        // Replace the SSID key with the stored SSID
        *htmlCodeBuffer = searchAndReplace(*htmlCodeBuffer, "%001%", ssid);
    }
    else
    {
        *htmlCodeBuffer = searchAndReplace(*htmlCodeBuffer, "%001%", "%NOT SET%");
    }

    if (passLen != 0)
    {   
        // Replace the password key with the stored password
        *htmlCodeBuffer = searchAndReplace(*htmlCodeBuffer, "%002%", password);
    }
    else
    {
        *htmlCodeBuffer = searchAndReplace(*htmlCodeBuffer, "%002%", "%NOT SET%");
    }
}

void fillMqttHtmlTmpl(char **htmlCodeBuffer)
{
    mqttSettings_t mqttSettings;
    esp_err_t err = loadMqttSettings(&mqttSettings);

    if (err != ESP_OK && err != ESP_ERR_NOT_FOUND) // We dont care if the settings are not found, we will just use the defaults
    {
        return;
    }

    //Fill in and replace the keys in the HTML template

    if (mqttSettings.addressLen != 0)
    {   
        // Replace the address key with the stored address
        *htmlCodeBuffer = searchAndReplace(*htmlCodeBuffer, "%001%", mqttSettings.address);
    }
    else
    {
        *htmlCodeBuffer = searchAndReplace(*htmlCodeBuffer, "%001%", "%NOT SET%");
    }

    if (mqttSettings.usernameLen != 0)
    {   
        // Replace the username key with the stored username
        *htmlCodeBuffer = searchAndReplace(*htmlCodeBuffer, "%002%", mqttSettings.username);
    }
    else
    {
        *htmlCodeBuffer = searchAndReplace(*htmlCodeBuffer, "%002%", "%NOT SET%");
    }

    if (mqttSettings.passwordLen != 0)
    {   
        // Replace the password key with the stored password
        *htmlCodeBuffer = searchAndReplace(*htmlCodeBuffer, "%003%", mqttSettings.password);
    }
    else
    {
        *htmlCodeBuffer = searchAndReplace(*htmlCodeBuffer, "%003%", "%NOT SET%");
    }
}