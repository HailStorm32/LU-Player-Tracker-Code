#include "ledControl.h"
#include "FreeRTOS.h"
#include "freertos/queue.h"

QueueHandle_t mqttJsonQueue;

const char* LED_CTRL_LOG_TAG = "LED_Control";

int initLedControl()
{
    mqttJsonQueue = xQueueCreate(10, sizeof(msgInfoPtr_t));

    if(mqttJsonQueue == 0)
    {
        ESP_LOGE(LED_CTRL_LOG_TAG, "ERROR: Unable to create mqttJsonQueue");
        return 1;
    }
}