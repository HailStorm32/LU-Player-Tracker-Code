#include "ledControl.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "cJSON.h"

#define LED_UPDATE_TASK_STACK_SIZE 2048 //Bytes

#define NUM_OF_WORLD_IDS 32 //All main worlds, minigames, side worlds, and 1 catch all for an unknown world ID

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

    xTaskCreate(ledUpdateTask, "led_update_task", LED_UPDATE_TASK_STACK_SIZE, NULL, 12, NULL);

    return 0;
}

void ledUpdateTask()
{
    msgInfoPtr_t jsonMsgInfo;
    cJSON* parsedWorldStatuses;

    while (true)
    {
       //Check for new JSON
       if (xQueueReceive(mqttJsonQueue, &jsonMsgInfo, (TickType_t)25))
       {
            ESP_LOGD(LED_CTRL_LOG_TAG, "OUT: \n %s\n\n", jsonMsgInfo->msgPtr);
           
            //Parse the JSON
            parsedWorldStatuses = cJSON_Parse(jsonMsgInfo->msgPtr);

            //Free the unparsed JSON struct that came in the queue
            free(jsonMsgInfo->msgPtr);
            free(jsonMsgInfo);
            jsonMsgInfo = NULL;
            
            //TODO:
            // Cycle through the total number of worlds
            // and check each world ID from the JSON

       }
    }
    
    //If we for whatever reason exit the loop, we need to close the task
    //free(receivedData);
    vTaskDelete(NULL);
}