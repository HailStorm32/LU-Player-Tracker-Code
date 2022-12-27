#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "../../credentials.h"

const uint8_t WIFI_MAX_RETRY = 8; //Max times wifi will try to reconnect before failing

static EventGroupHandle_t wifiEventGroup; //Create variable to hold event group that will contain wifi bit flags

//Define the bits we want to use for flags
#define WIFI_CONNECTED_BIT (1<<0) //Bit 0
#define WIFI_FAIL_BIT (1<<1) //Bit 1

static uint8_t wifiRetryNum = 0; //Keep track of the number of times we have tried to connect to wifi


//Handle incoming events for wifi and IP
static void eventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    if (eventBase == WIFI_EVENT)
    {
        switch (eventId) //(wifi_event_t)
        {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                if (wifiRetryNum < WIFI_MAX_RETRY)
                {
                    esp_wifi_connect();
                    wifiRetryNum++;
                    ESP_LOGI("WIFI", "retrying to connect to the AP");
                }
                else
                {
                    ESP_LOGI("WIFI", "connect to the AP fail");
                    xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT); //Set the bit 1 in the event group we created
                }
                break;
        }
    }
    if (eventBase == IP_EVENT)
    {
		switch (eventId) //(ip_event_t)
		{
			case IP_EVENT_STA_GOT_IP:
				ip_event_got_ip_t* event = (ip_event_got_ip_t*)eventData;
				ESP_LOGI("IP", "got IP:" IPSTR, IP2STR(&event->ip_info.ip));
				wifiRetryNum = 0;
				xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT); //Set the bit 0 in the event group we created
		}
	}
}

//Initalize wifi into station mode
void InitWifiSta(void)
{
    wifiEventGroup = xEventGroupCreate(); //Create and store the event group

    esp_netif_init(); //Initialize the lwIP TCP/IP stack 

    nvs_flash_init(); //Initialize flash to store wifi credentials

    esp_event_loop_create_default(); //Create a default event loop to listen for events

    esp_netif_create_default_wifi_sta(); //Setup lwIP TCP/IP stack for wifi station mode

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //Create initialization config for wifi and initialize it to all defaults

    esp_wifi_init(&cfg); //Initialize wifi

    //Handles to the instance event listener instance defined below
    esp_event_handler_instance_t instanceAnyId;
    esp_event_handler_instance_t instanceGotIp;

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &eventHandler, NULL, &instanceAnyId);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &eventHandler, NULL, &instanceGotIp);

   wifi_config_t wifiConfig = {
        .sta = {
            .ssid = WIFI_SSID, 
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    esp_wifi_start();

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI("WIFI", "Connected to ap SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI("WIFI", "Failed to connect to SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
    }
    else
    {
        ESP_LOGE("WIFI", "UNEXPECTED EVENT");
    }
}