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

#define TWDT_TIMEOUT_MS         3000
#define TASK_RESET_PERIOD_MS    2000
#define MAIN_DELAY_MS           10000

int app_main(void)
{
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

    initGPIO();
    initLedControl();

    if(gpio_get_level(GPIO_MODE_BTN) == HIGH)
    {
        changeSevenSegment(255, false);
        initWifiAP();
        vTaskDelay(12000);
    }
    
    initWifiSta();
    initMqttClient();

    printf("This is a test\n\n This is a Test2\n\n");
    int test = 10;
    printf("%d", test);
    return 0;
}