#include <ctype.h>
#include <string.h>

#include "htmlFill.h"
#include "flashStorage.h"


#define TEMPLATE_KEY_LEN    5

#define WIFI_NUM_KEYS       1
#define MQTT_NUM_KEYS       0
#define LED_NUM_KEYS        0

//TODO: Verify that this works as intended
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

void fillWifiHtml(char *htmlCodeBuffer)
{
    uint32_t length = strlen(htmlCodeBuffer);

    // Check to see if there is a stored SSID and password
    char ssid[SSID_MAX_LEN];
    char password[PASS_MAX_LEN];
    uint8_t ssidLen = 0;
    uint8_t passLen = 0;

    esp_err_t err = loadWifiCredentials(ssid, password, &ssidLen, &passLen);

    // If there is a stored SSID and password, fill the HTML code with it
    if (err == ESP_OK && ssidLen != 0)
    {   
        // Copy the HTML code to a new temporary string
        char htmlCodeCopy[length];
        strcpy(htmlCodeCopy, htmlCodeBuffer);

        // Determine the new size of the HTML code and reallocate the buffer
        uint32_t newLength = (length - TEMPLATE_KEY_LEN) + ssidLen;
        
        if(newLength > length)
        {
            //TODO: realocate buffer, then search, move the string up, then replace
        }
        else
        {
            //TODO: search and replace, move the string down, then realocate buffer
        }



    }





}