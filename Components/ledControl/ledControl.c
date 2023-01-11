#include "ledControl.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "cJSON.h"

#define LED_UPDATE_TASK_STACK_SIZE 2048 //Bytes

#define NUM_OF_WORLD_IDS 32 //All main worlds, minigames, side worlds, and 1 catch all for an unknown world ID

QueueHandle_t mqttJsonQueue;

const uint16_t auxWorldIDs[] = {
        1101, 1102, 1001, 1150, 1151, 1203, 1204, 1250, 1251, 1302, 1303, 1350, 1402, 1403, 1450, 2001
};

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

bool isAuxWorld(uint16_t worldID)
{
    uint8_t sizeOfArray = sizeof(auxWorldIDs) / sizeof(auxWorldIDs[0]);

    for(uint8_t index = 0; index < sizeOfArray; index++)
    {
        if(worldID == auxWorldIDs[index])
        {
            return true;
        }
    }
    return false;
}

void ledUpdateTask()
{
    msgInfoPtr_t jsonMsgInfo;
    cJSON* parsedWorldStatuses;
    cJSON* worldID;
    cJSON* worldName;
    cJSON* worldPop;

    uint8_t totalUniversePop = 0;
    bool auxWorldOccupied = false;

    while (true)
    {
       //Check for new JSON
       if (xQueueReceive(mqttJsonQueue, &jsonMsgInfo, (TickType_t)25))
       {
            ESP_LOGD(LED_CTRL_LOG_TAG, "JSON from queue: \n %s\n\n", jsonMsgInfo->msgPtr);
           
            //Parse the JSON
            parsedWorldStatuses = cJSON_Parse(jsonMsgInfo->msgPtr);

            //Report if there is an error in the JSON
            if (parsedWorldStatuses == NULL)
            {
                ESP_LOGE(LED_CTRL_LOG_TAG, "ERROR: Failed to parse JSON");
                
                const char* error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL)
                {
                    ESP_LOGE(LED_CTRL_LOG_TAG, "|__Error is before: %s", error_ptr);
                }
                continue;
            }

            //Free the unparsed JSON struct that came in the queue
            free(jsonMsgInfo->msgPtr);
            free(jsonMsgInfo);
            jsonMsgInfo = NULL;
            
            //Cycle through all the world ID objects
            cJSON_ArrayForEach(worldID, parsedWorldStatuses)
            {
                //Make sure the JSON is formated like we expect
                if(!cJSON_IsObject(worldID))
                {
                    ESP_LOGE(LED_CTRL_LOG_TAG, "ERROR: Expected different JSON");
                    
                    //Move to next item
                    continue;
                }

                //Get world name and population
                worldName = cJSON_GetObjectItemCaseSensitive(worldID, "name");
                worldPop = cJSON_GetObjectItemCaseSensitive(worldID, "pop");

                //Error check
                if(!cJSON_IsNumber(worldPop))
                {
                    ESP_LOGE(LED_CTRL_LOG_TAG, "ERROR: Expected different JSON\n|_Expected a number for worldPop, got somthing else");
                    
                    //Move to next item
                    continue;
                }
                if(!cJSON_IsString(worldName))
                {
                    ESP_LOGE(LED_CTRL_LOG_TAG, "ERROR: Expected different JSON\n|_Expected a string for worldName, got somthing else");
                    
                    //Move to next item
                    continue;
                }

                //Warn if there is an overflow on population
                if((totalUniversePop + worldPop->valueint) < totalUniversePop)
                {
                    ESP_LOGW(LED_CTRL_LOG_TAG, "WARNING: Potental overflow on total playercount (totalUniversePop)");
                }

                //Update universe population
                totalUniversePop += worldPop->valueint;

                //Update each world LED
                switch (atoi(worldID->string))
                {
                case 1000: //Venture Explorer
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Venture Explorer, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                case 1100: case 1101: case 1102: case 1001: case 1150: case 1151: //Avant Gardens, AG Survival, Spider Queen Battle, Return to VE, Block Yard, Avant Grove
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Avant Gardens, worldID triggered: %s", worldID->string);

                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED

                        //Update the aux world indicator if its an aux world
                        if(isAuxWorld(atoi(worldID->string)))
                        {
                            auxWorldOccupied = true;
                        }
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED

                    }
                    break;
                case 1200: case 1203: case 1204: case 1250: case 1251: //Nimbus Station, Vertigo Loop Racetrack, Battle of NS, Nimbus Rock, Nimbus Isle
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Nimbus Station, worldID triggered: %s", worldID->string);

                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED

                        //Update the aux world indicator if its an aux world
                        if(isAuxWorld(atoi(worldID->string)))
                        {
                            auxWorldOccupied = true;
                        }
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED

                    }
                    break;
                case 1201: //Pet Cove
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Pet Cove, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                case 1300: case 1302: case 1303: case 1350: //Gnarled Forest, Canyon Cove, Keelhaul Canyon, Chantey Shantey
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Gnarled Forest, worldID triggered: %s", worldID->string);

                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED

                        //Update the aux world indicator if its an aux world
                        if(isAuxWorld(atoi(worldID->string)))
                        {
                            auxWorldOccupied = true;
                        }
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                case 1400: case 1402: case 1403: case 1450: //Forbidden Valley, FV Dragon, Dragonmaw Chasm, Raven Bluff
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Forbidden Valley, worldID triggered: %s", worldID->string);

                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED

                        //Update the aux world indicator if its an aux world
                        if(isAuxWorld(atoi(worldID->string)))
                        {
                            auxWorldOccupied = true;
                        }
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                case 1600: //Starbase 3001
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Starbase 3001, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                case 1601: //Deep Freeze
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Deep Freeze, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                case 1602: //Robot City
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Robot City, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                case 1603: //Moon Base
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Moon Base, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                case 1604: //Portabello
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Portabello, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                case 1700: //LEGO Club (Club Station Alpha)
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: LEGO Club (Club Station Alpha), worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                case 1800: //Crux Prime
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Crux Prime, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                case 1900: //Nexus Tower
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Nexus Tower, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                case 2000: case 2001: //Ninjago, Frakjaw Battle
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Ninjago, worldID triggered: %s", worldID->string);

                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        //TODO:
                        // Turn on world LED

                        //Update the aux world indicator if its an aux world
                        if(isAuxWorld(atoi(worldID->string)))
                        {
                            auxWorldOccupied = true;
                        }
                    }
                    else
                    {
                        //TODO:
                        // Turn off world LED
                    }
                    break;
                
                default: //Unsupported World ID
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Unsupported World ID, no LED to update, worldID triggered: %s", worldID->string);
                    break;
                }
            }

            //TODO:
            // Send updated universe population to seven segment display updater

            //Reset the universe population
            totalUniversePop = 0;
            auxWorldOccupied = false;
            
            //Recursively free the parsed JSON
            cJSON_Delete(parsedWorldStatuses);
            parsedWorldStatuses = NULL;
            worldID = NULL;
            worldName = NULL;
            worldPop = NULL;
       }
    }
    
    //If we for whatever reason exit the loop, we need to close the task
    //free(receivedData);
    vTaskDelete(NULL);
}

/* void freeItem(cJSON *item) {
    if ((item != NULL) && (item->child != NULL))
    {
        cJSON_Delete(item->child);
    }
    if ((item->valuestring != NULL) && !(item->type & cJSON_IsReference))
    {
        free(item->valuestring);
    }
    if ((item->string != NULL) && !(item->type & cJSON_StringIsConst))
    {
        free(item->string);
    }
} */
