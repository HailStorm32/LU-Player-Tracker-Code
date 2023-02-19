#include <string.h>
#include "ledControl.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "cJSON.h"
#include "driver/rmt_tx.h"
#include "freertos/task.h"
#include "iPixel.h"
#include "esp_err.h"
#include "esp_task_wdt.h"
#include "sevenSegmentControl.h"

#define LED_UPDATE_TASK_STACK_SIZE 4096 //Bytes 2048

#define NUM_OF_SUPORTED_WRLD_IDS 34 //All main worlds, minigames, side worlds, and 1 catch all for an unknown world ID

#define NUM_OF_LEDS 15
#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us
#define RMT_LED_STRIP_GPIO_NUM      9

#define UNIQUE_WORLD_COLORS true

#define LED_OFF 0

rmt_channel_handle_t led_chan;
rmt_encoder_handle_t led_encoder;
rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no transfer loop
    };

uint8_t led_strip_pixels[NUM_OF_LEDS * 3];

QueueHandle_t mqttJsonQueue;

struct worldLed
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t ledArrayIndexStart;
}veColor, agColor, nsColor, petCoveColor, gfColor, fbColor, lClubColor, 
    ntColor, nijagoColor, cpColor, portabelloColor, moonbaseColor, robotCityColor, deepFreezeColor, starbaseColor;

struct worldLed* worldLedStructArray[NUM_OF_LEDS] = {&veColor, &agColor, &petCoveColor, &gfColor, &fbColor, &nsColor, &lClubColor, 
        &ntColor, &nijagoColor, &cpColor, &portabelloColor, &moonbaseColor, &robotCityColor, &deepFreezeColor, &starbaseColor};

enum WRLD_INDEX {VE, AG, PC, GF, FV, NS, LC, NT, NINJA, CP, PORT, MOON, ROBOT, DEEP, STAR};


#define MASTER_WRLD_COLOR_RED 18
#define MASTER_WRLD_COLOR_GREEN 55
#define MASTER_WRLD_COLOR_BLUE 201
#define MASTER_BRIGHTNESS .5

const uint16_t auxWorldIDs[] = {
        1101, 1102, 1001, 1150, 1151, 1203, 1204, 1250, 1251, 1302, 1303, 1350, 1402, 1403, 1450, 2001, 1260, 1261
};

const char* LED_CTRL_LOG_TAG = "LED_Control";

int initLedControl()
{
    //Set the array index for each world LED
    for(uint8_t ledIndex = 0; ledIndex < NUM_OF_LEDS; ledIndex++)
    {
        worldLedStructArray[ledIndex]->ledArrayIndexStart = ledIndex * 3;
    }
    
    //World LED options
    if (UNIQUE_WORLD_COLORS)
    {
        //Venture explore
        float veBrightness = .5;
        veColor.red = 18 * veBrightness;
        veColor.green = 55 * veBrightness;
        veColor.blue = 201 * veBrightness;

        //Avant Gardens
        float agBrightness = .5;
        agColor.red = 245 * agBrightness;
        agColor.green = 72 * agBrightness;
        agColor.blue = 24 * agBrightness;
        
        //Pet Cove
        float petCoveBrightness = .5;
        petCoveColor.red = 22 * petCoveBrightness;
        petCoveColor.green = 186 * petCoveBrightness;
        petCoveColor.blue = 123 * petCoveBrightness;

        //Gnarled Forest
        float gfBrightness = .5;
        gfColor.red = 91 * gfBrightness;
        gfColor.green = 186 * gfBrightness;
        gfColor.blue = 22 * gfBrightness;

        //Forbidden Valley
        float fvBrightness = .5;
        fbColor.red = 230 * fvBrightness;
        fbColor.green = 48 * fvBrightness;
        fbColor.blue = 48 * fvBrightness;
        
        //Nimbus Station
        float nsBrightness = .5;
        nsColor.red = 18 * nsBrightness;
        nsColor.green = 55 * nsBrightness;
        nsColor.blue = 201 * nsBrightness;

        //Club Station Alpha
        float lClubBrightness = .5;
        lClubColor.red = 18 * lClubBrightness;
        lClubColor.green = 55 * lClubBrightness;
        lClubColor.blue = 201 * lClubBrightness;

        //Nexus Tower
        float ntBrightness = .5;
        ntColor.red = 40 * ntBrightness;
        ntColor.green = 222 * ntBrightness;
        ntColor.blue = 216 * ntBrightness;

        //Ninjago
        float ninjagoBrightness = .5;
        nijagoColor.red = 186 * ninjagoBrightness;
        nijagoColor.green = 24 * ninjagoBrightness;
        nijagoColor.blue = 245 * ninjagoBrightness;

        //Crux Prime
        float cpBrightness = .5;
        cpColor.red = 79 * cpBrightness;
        cpColor.green = 39 * cpBrightness;
        cpColor.blue = 227 * cpBrightness;
        
        //Portabello
        float portabelloBrightness = .5;
        portabelloColor.red = 173 * portabelloBrightness;
        portabelloColor.green = 29 * portabelloBrightness;
        portabelloColor.blue = 133 * portabelloBrightness;

        //Moonbase
        float moonbaseBrightness = .5;
        moonbaseColor.red = 107 * moonbaseBrightness;
        moonbaseColor.green = 106 * moonbaseBrightness;
        moonbaseColor.blue = 107 * moonbaseBrightness;

        //Robot City
        float robotCityBrightness = .5;
        robotCityColor.red = 247 * robotCityBrightness;
        robotCityColor.green = 248 * robotCityBrightness;
        robotCityColor.blue = 250 * robotCityBrightness;

        //Deep Freeze
        float deepFreezeBrightness = .5;
        deepFreezeColor.red = 61 * deepFreezeBrightness;
        deepFreezeColor.green = 113 * deepFreezeBrightness;
        deepFreezeColor.blue = 245 * deepFreezeBrightness;

        //Starbase 3001
        float starbaseBrightness = .5;
        starbaseColor.red = 18 * starbaseBrightness;
        starbaseColor.green = 55 * starbaseBrightness;
        starbaseColor.blue = 201 * starbaseBrightness;
        

    }
    else
    {
        //Set all world LEDs to the same color
        for (uint8_t ledIndex = 0; ledIndex < NUM_OF_LEDS; ledIndex++)
        {
            worldLedStructArray[ledIndex]->red = MASTER_WRLD_COLOR_RED * MASTER_BRIGHTNESS;
            worldLedStructArray[ledIndex]->green = MASTER_WRLD_COLOR_GREEN * MASTER_BRIGHTNESS;
            worldLedStructArray[ledIndex]->blue = MASTER_WRLD_COLOR_BLUE * MASTER_BRIGHTNESS;
        }
    }

    mqttJsonQueue = xQueueCreate(10, sizeof(msgInfoPtr_t));

    if(mqttJsonQueue == 0)
    {
        ESP_LOGE(LED_CTRL_LOG_TAG, "ERROR: Unable to create mqttJsonQueue");
        return 1;
    }

    //Setup iPixel Driver
    ESP_LOGI(LED_CTRL_LOG_TAG, "Create RMT TX channel");
    led_chan = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .gpio_num = RMT_LED_STRIP_GPIO_NUM,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(LED_CTRL_LOG_TAG, "Install led strip encoder");
    led_encoder = NULL;
    led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    ESP_LOGI(LED_CTRL_LOG_TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));
    
    //Initalize LEDs to all off
    memset(led_strip_pixels, 0, sizeof(led_strip_pixels));
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
    
    xTaskCreatePinnedToCore(ledUpdateTask, "led_update_task", LED_UPDATE_TASK_STACK_SIZE, NULL, 12, NULL, 1);

    //Initialize seven segment display
    initSevenSegment();

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

    // //Subscribe to the watchdog
    // esp_task_wdt_add(NULL);
    // ESP_ERROR_CHECK(esp_task_wdt_status(NULL));

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

                //Free the unparsed JSON struct that came in the queue
                free(jsonMsgInfo->msgPtr);
                free(jsonMsgInfo);
                jsonMsgInfo = NULL;

                ESP_LOGE(LED_CTRL_LOG_TAG, "Skipped bad JSON");

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
                        //Turn on world LED
                        led_strip_pixels[worldLedStructArray[VE]->ledArrayIndexStart + 0] = worldLedStructArray[VE]->green;
                        led_strip_pixels[worldLedStructArray[VE]->ledArrayIndexStart + 1] = worldLedStructArray[VE]->red;
                        led_strip_pixels[worldLedStructArray[VE]->ledArrayIndexStart + 2] = worldLedStructArray[VE]->blue;
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[VE]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[VE]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[VE]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                case 1100: case 1101: case 1102: case 1001: case 1150: case 1151: //Avant Gardens, AG Survival, Spider Queen Battle, Return to VE, Block Yard, Avant Grove
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Avant Gardens, worldID triggered: %s", worldID->string);

                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[AG]->ledArrayIndexStart + 0] = worldLedStructArray[AG]->green;
                        led_strip_pixels[worldLedStructArray[AG]->ledArrayIndexStart + 1] = worldLedStructArray[AG]->red;
                        led_strip_pixels[worldLedStructArray[AG]->ledArrayIndexStart + 2] = worldLedStructArray[AG]->blue;

                        //Update the aux world indicator if its an aux world
                        if(isAuxWorld(atoi(worldID->string)))
                        {
                            auxWorldOccupied = true;
                        }
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[AG]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[AG]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[AG]->ledArrayIndexStart + 2] = LED_OFF;

                    } */
                    break;
                case 1200: case 1203: case 1204: case 1250: case 1251: case 1260: case 1261: //Nimbus Station, Vertigo Loop Racetrack, Battle of NS, Nimbus Rock, Nimbus Isle, Frostburgh, Frostburgh race track
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Nimbus Station, worldID triggered: %s", worldID->string);

                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[NS]->ledArrayIndexStart + 0] = worldLedStructArray[NS]->green;
                        led_strip_pixels[worldLedStructArray[NS]->ledArrayIndexStart + 1] = worldLedStructArray[NS]->red;
                        led_strip_pixels[worldLedStructArray[NS]->ledArrayIndexStart + 2] = worldLedStructArray[NS]->blue;

                        //Update the aux world indicator if its an aux world
                        if(isAuxWorld(atoi(worldID->string)))
                        {
                            auxWorldOccupied = true;
                        }
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[NS]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[NS]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[NS]->ledArrayIndexStart + 2] = LED_OFF;

                    } */
                    break;
                case 1201: //Pet Cove
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Pet Cove, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[PC]->ledArrayIndexStart + 0] = worldLedStructArray[PC]->green;
                        led_strip_pixels[worldLedStructArray[PC]->ledArrayIndexStart + 1] = worldLedStructArray[PC]->red;
                        led_strip_pixels[worldLedStructArray[PC]->ledArrayIndexStart + 2] = worldLedStructArray[PC]->blue;
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[PC]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[PC]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[PC]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                case 1300: case 1302: case 1303: case 1350: //Gnarled Forest, Cannon Cove, Keelhaul Canyon, Chantey Shantey
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Gnarled Forest, worldID triggered: %s", worldID->string);

                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[GF]->ledArrayIndexStart + 0] = worldLedStructArray[GF]->green;
                        led_strip_pixels[worldLedStructArray[GF]->ledArrayIndexStart + 1] = worldLedStructArray[GF]->red;
                        led_strip_pixels[worldLedStructArray[GF]->ledArrayIndexStart + 2] = worldLedStructArray[GF]->blue;

                        //Update the aux world indicator if its an aux world
                        if(isAuxWorld(atoi(worldID->string)))
                        {
                            auxWorldOccupied = true;
                        }
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[GF]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[GF]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[GF]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                case 1400: case 1402: case 1403: case 1450: //Forbidden Valley, FV Dragon, Dragonmaw Chasm, Raven Bluff
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Forbidden Valley, worldID triggered: %s", worldID->string);

                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[FV]->ledArrayIndexStart + 0] = worldLedStructArray[FV]->green;
                        led_strip_pixels[worldLedStructArray[FV]->ledArrayIndexStart + 1] = worldLedStructArray[FV]->red;
                        led_strip_pixels[worldLedStructArray[FV]->ledArrayIndexStart + 2] = worldLedStructArray[FV]->blue;


                        //Update the aux world indicator if its an aux world
                        if(isAuxWorld(atoi(worldID->string)))
                        {
                            auxWorldOccupied = true;
                        }
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[FV]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[FV]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[FV]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                case 1600: //Starbase 3001
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Starbase 3001, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[STAR]->ledArrayIndexStart + 0] = worldLedStructArray[STAR]->green;
                        led_strip_pixels[worldLedStructArray[STAR]->ledArrayIndexStart + 1] = worldLedStructArray[STAR]->red;
                        led_strip_pixels[worldLedStructArray[STAR]->ledArrayIndexStart + 2] = worldLedStructArray[STAR]->blue;
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[STAR]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[STAR]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[STAR]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                case 1601: //Deep Freeze
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Deep Freeze, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[DEEP]->ledArrayIndexStart + 0] = worldLedStructArray[DEEP]->green;
                        led_strip_pixels[worldLedStructArray[DEEP]->ledArrayIndexStart + 1] = worldLedStructArray[DEEP]->red;
                        led_strip_pixels[worldLedStructArray[DEEP]->ledArrayIndexStart + 2] = worldLedStructArray[DEEP]->blue;
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[DEEP]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[DEEP]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[DEEP]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                case 1602: //Robot City
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Robot City, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[ROBOT]->ledArrayIndexStart + 0] = worldLedStructArray[ROBOT]->green;
                        led_strip_pixels[worldLedStructArray[ROBOT]->ledArrayIndexStart + 1] = worldLedStructArray[ROBOT]->red;
                        led_strip_pixels[worldLedStructArray[ROBOT]->ledArrayIndexStart + 2] = worldLedStructArray[ROBOT]->blue;
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[ROBOT]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[ROBOT]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[ROBOT]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                case 1603: //Moon Base
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Moon Base, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[MOON]->ledArrayIndexStart + 0] = worldLedStructArray[MOON]->green;
                        led_strip_pixels[worldLedStructArray[MOON]->ledArrayIndexStart + 1] = worldLedStructArray[MOON]->red;
                        led_strip_pixels[worldLedStructArray[MOON]->ledArrayIndexStart + 2] = worldLedStructArray[MOON]->blue;
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[MOON]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[MOON]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[MOON]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                case 1604: //Portabello
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Portabello, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[PORT]->ledArrayIndexStart + 0] = worldLedStructArray[PORT]->green;
                        led_strip_pixels[worldLedStructArray[PORT]->ledArrayIndexStart + 1] = worldLedStructArray[PORT]->red;
                        led_strip_pixels[worldLedStructArray[PORT]->ledArrayIndexStart + 2] = worldLedStructArray[PORT]->blue;
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[PORT]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[PORT]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[PORT]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                case 1700: //LEGO Club (Club Station Alpha)
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: LEGO Club (Club Station Alpha), worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[LC]->ledArrayIndexStart + 0] = worldLedStructArray[LC]->green;
                        led_strip_pixels[worldLedStructArray[LC]->ledArrayIndexStart + 1] = worldLedStructArray[LC]->red;
                        led_strip_pixels[worldLedStructArray[LC]->ledArrayIndexStart + 2] = worldLedStructArray[LC]->blue;
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[LC]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[LC]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[LC]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                case 1800: //Crux Prime
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Crux Prime, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[CP]->ledArrayIndexStart + 0] = worldLedStructArray[CP]->green;
                        led_strip_pixels[worldLedStructArray[CP]->ledArrayIndexStart + 1] = worldLedStructArray[CP]->red;
                        led_strip_pixels[worldLedStructArray[CP]->ledArrayIndexStart + 2] = worldLedStructArray[CP]->blue;
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[CP]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[CP]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[CP]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                case 1900: //Nexus Tower
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Nexus Tower, worldID triggered: %s", worldID->string);
                    
                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[NT]->ledArrayIndexStart + 0] = worldLedStructArray[NT]->green;
                        led_strip_pixels[worldLedStructArray[NT]->ledArrayIndexStart + 1] = worldLedStructArray[NT]->red;
                        led_strip_pixels[worldLedStructArray[NT]->ledArrayIndexStart + 2] = worldLedStructArray[NT]->blue;
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[NT]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[NT]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[NT]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                case 2000: case 2001: //Ninjago, Frakjaw Battle
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Updating LED: Ninjago, worldID triggered: %s", worldID->string);

                    //Turn on the world LED if its occupied
                    if(worldPop->valueint > 0)
                    {
                        // Turn on world LED
                        led_strip_pixels[worldLedStructArray[NINJA]->ledArrayIndexStart + 0] = worldLedStructArray[NINJA]->green;
                        led_strip_pixels[worldLedStructArray[NINJA]->ledArrayIndexStart + 1] = worldLedStructArray[NINJA]->red;
                        led_strip_pixels[worldLedStructArray[NINJA]->ledArrayIndexStart + 2] = worldLedStructArray[NINJA]->blue;

                        //Update the aux world indicator if its an aux world
                        if(isAuxWorld(atoi(worldID->string)))
                        {
                            auxWorldOccupied = true;
                        }
                    }
                    /* else
                    {
                        // Turn off world LED
                        led_strip_pixels[worldLedStructArray[NINJA]->ledArrayIndexStart + 0] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[NINJA]->ledArrayIndexStart + 1] = LED_OFF;
                        led_strip_pixels[worldLedStructArray[NINJA]->ledArrayIndexStart + 2] = LED_OFF;
                    } */
                    break;
                
                default: //Unsupported World ID
                    ESP_LOGI(LED_CTRL_LOG_TAG, "Unsupported World ID, no LED to update, worldID triggered: %s", worldID->string);
                    break;
                }
            }
            
            //Update LEDs
            ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
            vTaskDelay(pdMS_TO_TICKS(100)); //Give the driver time to send data before modifying the array
            

            // Send updated universe population to seven segment display updater
            changeSevenSegment(totalUniversePop, auxWorldOccupied);

            //Reset the universe population
            totalUniversePop = 0;
            auxWorldOccupied = false;

            //Reset LED world States
            memset(led_strip_pixels, 0, sizeof(led_strip_pixels));
            
            //Recursively free the parsed JSON
            cJSON_Delete(parsedWorldStatuses);
            parsedWorldStatuses = NULL;
            worldID = NULL;
            worldName = NULL;
            worldPop = NULL;
       }
       //vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    //If we for whatever reason exit the loop, we need to close the task
    vTaskDelete(NULL);
}
