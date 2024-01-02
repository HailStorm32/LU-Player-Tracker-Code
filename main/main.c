#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <stdio.h>
#include "esp_log.h"
#include "mqtt.h"
#include "wifi.h"
#include "ledControl.h"
#include "sevenSegmentControl.h"
#include "gpioControl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_task_wdt.h"
#include "httpServer.h"
#include "flashStorage.h"

#define TWDT_TIMEOUT_MS         3000
#define TASK_RESET_PERIOD_MS    2000
#define MAIN_DELAY_MS           10000

int app_main(void)
{
    esp_err_t err = ESP_OK;

    esp_task_wdt_deinit();
    // // If the TWDT was not initialized automatically on startup, manually intialize it now
    // esp_task_wdt_config_t twdt_config = {
    //     .timeout_ms = TWDT_TIMEOUT_MS,
    //     .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,    // TODO: Figure out what bit mask results in only core 0
    //     .trigger_panic = false,
    // };
    // ESP_ERROR_CHECK(esp_task_wdt_init(&twdt_config));
    // printf("TWDT initialized\n");
    

    esp_log_level_set("MQTT_Control", ESP_LOG_DEBUG); //ESP_LOG_DEBUG
    esp_log_level_set("Linked_List", ESP_LOG_WARN);  //ESP_LOG_INFO
    esp_log_level_set("LED_Control", ESP_LOG_DEBUG);  //ESP_LOG_DEBUG
    esp_log_level_set("Segment_Update", ESP_LOG_DEBUG);  //ESP_LOG_DEBUG
    esp_log_level_set("Flash_Storage", ESP_LOG_DEBUG);  //ESP_LOG_DEBUG
    esp_log_level_set("static_page", ESP_LOG_DEBUG);  //ESP_LOG_DEBUG
    esp_log_level_set("WIFI", ESP_LOG_DEBUG);  //ESP_LOG_DEBUG

    // Initialize GPIO, LED, and Flash Storage
    if((err = initGPIO()) != ESP_OK || (err = initLedControl()) != ESP_OK || (err = initFlashStorage()) != ESP_OK)
    {
        //TODO: Implement better error handling
        ESP_ERROR_CHECK(err);
    }

    
    // Initialize Wifi and MQTT, if either fails or the mode button is pressed, initialize AP mode and HTTP server and have the user configure the device
    if((gpio_get_level(GPIO_MODE_BTN) == HIGH) || ((err = initWifiSta()) != ESP_OK || (err = initMqttClient()) != ESP_OK))
    {
        //TODO: Log the error

        changeSevenSegment(255, false);
        initWifiAP();
        initHttpServer();
        while (true)
        {
            vTaskDelay(120);// TODO: Implement way for user to quit AP mode instead of forcing them to reboot
        }
    }

    printf("Boot successful\n");
    return 0;
}