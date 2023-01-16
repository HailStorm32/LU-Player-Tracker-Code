#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <stdio.h>
#include "esp_log.h"
#include "mqtt.h"
#include "wifi.h"
#include "ledControl.h"

int app_main(void)
{
    esp_log_level_set("MQTT_Control", ESP_LOG_DEBUG); //ESP_LOG_DEBUG
    esp_log_level_set("Linked_List", ESP_LOG_INFO); 
    esp_log_level_set("LED_Control", ESP_LOG_DEBUG);  //ESP_LOG_DEBUG

    initLedControl();
    InitWifiSta();
    initMqttClient();

    printf("This is a test\n\n This is a Test2\n\n");
    int test = 10;
    printf("%d", test);
    return 0;
}