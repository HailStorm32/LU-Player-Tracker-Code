#include "sevenSegmentControl.h"
#include "gpioControl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>


/*               
                --A-                A == 0  |   E == 4
              F|_G_| B              B == 1  |   F == 5
             E|_D_|C                C == 2  |   G == 6
                    * H             D == 3  |   H == 7
*/              

#define NUM_OF_SEGMENTS 8
#define NUM_OF_DIGITS   2

#define ON              true
#define OFF             false

enum SEGMENTS {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, SEG_H};

//Array for holding display data. [digit][segment]
bool displaySegments[NUM_OF_DIGITS][NUM_OF_SEGMENTS];

bool numberSegments[11][NUM_OF_SEGMENTS];



void segmentTest()
{
    while(1)
{
   // for (int i =0; i < 8; i++)
   // {
        // gpio_set_level(GPIO_TO_MUX_A0, i & 1);
        // gpio_set_level(GPIO_TO_MUX_A1, (i>>1) & 1);
        // gpio_set_level(GPIO_TO_MUX_A2, (i>>2) & 1);

        gpio_set_level(GPIO_TO_MUX_A0, 0);
        gpio_set_level(GPIO_TO_MUX_A1, 0);
        gpio_set_level(GPIO_TO_MUX_A2, 0);

        vTaskDelay(pdMS_TO_TICKS(3000));

        gpio_set_level(GPIO_TO_MUX_A0, 1);
        gpio_set_level(GPIO_TO_MUX_A1, 0);
        gpio_set_level(GPIO_TO_MUX_A2, 0);

        vTaskDelay(pdMS_TO_TICKS(3000));

        gpio_set_level(GPIO_TO_MUX_A0, 0);
        gpio_set_level(GPIO_TO_MUX_A1, 1);
        gpio_set_level(GPIO_TO_MUX_A2, 0);

        vTaskDelay(pdMS_TO_TICKS(3000));

        gpio_set_level(GPIO_TO_MUX_A0, 1);
        gpio_set_level(GPIO_TO_MUX_A1, 1);
        gpio_set_level(GPIO_TO_MUX_A2, 0);

        vTaskDelay(pdMS_TO_TICKS(3000));

        gpio_set_level(GPIO_TO_MUX_A0, 0);
        gpio_set_level(GPIO_TO_MUX_A1, 0);
        gpio_set_level(GPIO_TO_MUX_A2, 1);

        vTaskDelay(pdMS_TO_TICKS(3000));

        gpio_set_level(GPIO_TO_MUX_A0, 1);
        gpio_set_level(GPIO_TO_MUX_A1, 0);
        gpio_set_level(GPIO_TO_MUX_A2, 1);

        vTaskDelay(pdMS_TO_TICKS(3000));

        gpio_set_level(GPIO_TO_MUX_A0, 0);
        gpio_set_level(GPIO_TO_MUX_A1, 1);
        gpio_set_level(GPIO_TO_MUX_A2, 1);

        vTaskDelay(pdMS_TO_TICKS(3000));

        gpio_set_level(GPIO_TO_MUX_A0, 1);
        gpio_set_level(GPIO_TO_MUX_A1, 1);
        gpio_set_level(GPIO_TO_MUX_A2, 1);

        vTaskDelay(pdMS_TO_TICKS(3000));
    //}
}
}

int initSevenSegment()
{
    //Define numbers

    //Digit 0
    numberSegments[0][SEG_A] = ON;
    numberSegments[0][SEG_B] = ON;
    numberSegments[0][SEG_C] = ON;
    numberSegments[0][SEG_D] = ON;
    numberSegments[0][SEG_E] = ON;
    numberSegments[0][SEG_F] = ON;
    numberSegments[0][SEG_G] = OFF;
    numberSegments[0][SEG_H] = OFF;

    //Digit 1
    numberSegments[1][SEG_A] = OFF;
    numberSegments[1][SEG_B] = ON;
    numberSegments[1][SEG_C] = ON;
    numberSegments[1][SEG_D] = OFF;
    numberSegments[1][SEG_E] = OFF;
    numberSegments[1][SEG_F] = OFF;
    numberSegments[1][SEG_G] = OFF;
    numberSegments[1][SEG_H] = OFF;

    //Digit 2
    numberSegments[2][SEG_A] = ON;
    numberSegments[2][SEG_B] = ON;
    numberSegments[2][SEG_C] = OFF;
    numberSegments[2][SEG_D] = OFF;
    numberSegments[2][SEG_E] = ON;
    numberSegments[2][SEG_F] = OFF;
    numberSegments[2][SEG_G] = ON;
    numberSegments[2][SEG_H] = OFF;

    //Digit 3
    numberSegments[3][SEG_A] = ON;
    numberSegments[3][SEG_B] = ON;
    numberSegments[3][SEG_C] = ON;
    numberSegments[3][SEG_D] = ON;
    numberSegments[3][SEG_E] = OFF;
    numberSegments[3][SEG_F] = OFF;
    numberSegments[3][SEG_G] = ON;
    numberSegments[3][SEG_H] = OFF;

    //Digit 4
    numberSegments[4][SEG_A] = OFF;
    numberSegments[4][SEG_B] = ON;
    numberSegments[4][SEG_C] = ON;
    numberSegments[4][SEG_D] = OFF;
    numberSegments[4][SEG_E] = OFF;
    numberSegments[4][SEG_F] = ON;
    numberSegments[4][SEG_G] = ON;
    numberSegments[4][SEG_H] = OFF;

    //Digit 5
    numberSegments[5][SEG_A] = ON;
    numberSegments[5][SEG_B] = OFF;
    numberSegments[5][SEG_C] = ON;
    numberSegments[5][SEG_D] = ON;
    numberSegments[5][SEG_E] = OFF;
    numberSegments[5][SEG_F] = ON;
    numberSegments[5][SEG_G] = ON;
    numberSegments[5][SEG_H] = OFF;

    //Digit 6
    numberSegments[6][SEG_A] = ON;
    numberSegments[6][SEG_B] = OFF;
    numberSegments[6][SEG_C] = ON;
    numberSegments[6][SEG_D] = ON;
    numberSegments[6][SEG_E] = ON;
    numberSegments[6][SEG_F] = ON;
    numberSegments[6][SEG_G] = ON;
    numberSegments[6][SEG_H] = OFF;

    //Digit 7
    numberSegments[7][SEG_A] = ON;
    numberSegments[7][SEG_B] = ON;
    numberSegments[7][SEG_C] = ON;
    numberSegments[7][SEG_D] = OFF;
    numberSegments[7][SEG_E] = OFF;
    numberSegments[7][SEG_F] = OFF;
    numberSegments[7][SEG_G] = OFF;
    numberSegments[7][SEG_H] = OFF;

    //Digit 8
    numberSegments[8][SEG_A] = ON;
    numberSegments[8][SEG_B] = ON;
    numberSegments[8][SEG_C] = ON;
    numberSegments[8][SEG_D] = ON;
    numberSegments[8][SEG_E] = ON;
    numberSegments[8][SEG_F] = ON;
    numberSegments[8][SEG_G] = ON;
    numberSegments[8][SEG_H] = OFF;

    //Digit 9
    numberSegments[9][SEG_A] = ON;
    numberSegments[9][SEG_B] = ON;
    numberSegments[9][SEG_C] = ON;
    numberSegments[9][SEG_D] = OFF;
    numberSegments[9][SEG_E] = OFF;
    numberSegments[9][SEG_F] = ON;
    numberSegments[9][SEG_G] = ON;
    numberSegments[9][SEG_H] = OFF;

    //Error (number too large)
    numberSegments[10][SEG_A] = OFF;
    numberSegments[10][SEG_B] = OFF;
    numberSegments[10][SEG_C] = OFF;
    numberSegments[10][SEG_D] = OFF;
    numberSegments[10][SEG_E] = OFF;
    numberSegments[10][SEG_F] = OFF;
    numberSegments[10][SEG_G] = ON;
    numberSegments[10][SEG_H] = OFF;

    // Preset all GPIO pins for seven segment display
    gpio_set_level(GPIO_TO_MUX_A0, LOW);
    gpio_set_level(GPIO_TO_MUX_A1, LOW);
    gpio_set_level(GPIO_TO_MUX_A2, LOW);
    gpio_set_level(GPIO_DIG_SEL, LEFT_DIGIT);

    // TODO:
    //  Start update task

    return 0;
}

void changeSevenSegment(uint8_t numberToDisplay, bool auxWorld)
{
    if(numberToDisplay < 10)
    {
        //Set right digit
        memcpy(displaySegments[RIGHT_DIGIT], numberSegments[numberToDisplay], sizeof(numberSegments[numberToDisplay]));

        //Set left digit -- 0
        memcpy(displaySegments[LEFT_DIGIT], numberSegments[0], sizeof(numberSegments[0]));
    }
    else if(numberToDisplay >= 10 && numberToDisplay < 20)
    {
        //Set right digit
        memcpy(displaySegments[RIGHT_DIGIT], numberSegments[numberToDisplay - 10], sizeof(numberSegments[numberToDisplay - 10]));

        //Set left digit -- 1
        memcpy(displaySegments[LEFT_DIGIT], numberSegments[1], sizeof(numberSegments[1]));
    }
    else if(numberToDisplay >= 20 && numberToDisplay < 30)
    {
        //Set right digit
        memcpy(displaySegments[RIGHT_DIGIT], numberSegments[numberToDisplay - 20], sizeof(numberSegments[numberToDisplay - 20]));

        //Set left digit -- 2
        memcpy(displaySegments[LEFT_DIGIT], numberSegments[2], sizeof(numberSegments[2]));
    }
    else if(numberToDisplay >= 30 && numberToDisplay < 40)
    {
        //Set right digit
        memcpy(displaySegments[RIGHT_DIGIT], numberSegments[numberToDisplay - 30], sizeof(numberSegments[numberToDisplay - 30]));

        //Set left digit -- 3
        memcpy(displaySegments[LEFT_DIGIT], numberSegments[3], sizeof(numberSegments[3]));
    }
    else if(numberToDisplay >= 40 && numberToDisplay < 50)
    {
        //Set right digit
        memcpy(displaySegments[RIGHT_DIGIT], numberSegments[numberToDisplay - 40], sizeof(numberSegments[numberToDisplay - 40]));

        //Set left digit -- 4
        memcpy(displaySegments[LEFT_DIGIT], numberSegments[4], sizeof(numberSegments[4]));
    }
    else if(numberToDisplay >= 50 && numberToDisplay < 60)
    {
        //Set right digit
        memcpy(displaySegments[RIGHT_DIGIT], numberSegments[numberToDisplay - 50], sizeof(numberSegments[numberToDisplay - 50]));

        //Set left digit -- 5
        memcpy(displaySegments[LEFT_DIGIT], numberSegments[5], sizeof(numberSegments[5]));
    }
    else if(numberToDisplay >= 60 && numberToDisplay < 70)
    {
        //Set right digit
        memcpy(displaySegments[RIGHT_DIGIT], numberSegments[numberToDisplay - 60], sizeof(numberSegments[numberToDisplay - 60]));

        //Set left digit -- 6
        memcpy(displaySegments[LEFT_DIGIT], numberSegments[6], sizeof(numberSegments[6]));
    }
    else if(numberToDisplay >= 70 && numberToDisplay < 80)
    {
        //Set right digit
        memcpy(displaySegments[RIGHT_DIGIT], numberSegments[numberToDisplay - 70], sizeof(numberSegments[numberToDisplay - 70]));

        //Set left digit -- 7
        memcpy(displaySegments[LEFT_DIGIT], numberSegments[7], sizeof(numberSegments[7]));
    }
    else if(numberToDisplay >= 80 && numberToDisplay < 90)
    {
        //Set right digit
        memcpy(displaySegments[RIGHT_DIGIT], numberSegments[numberToDisplay - 80], sizeof(numberSegments[numberToDisplay - 80]));

        //Set left digit -- 8
        memcpy(displaySegments[LEFT_DIGIT], numberSegments[8], sizeof(numberSegments[8]));
    }
    else if(numberToDisplay >= 90 && numberToDisplay < 100)
    {
        //Set right digit
        memcpy(displaySegments[RIGHT_DIGIT], numberSegments[numberToDisplay - 90], sizeof(numberSegments[numberToDisplay - 90]));

        //Set left digit -- 9
        memcpy(displaySegments[LEFT_DIGIT], numberSegments[9], sizeof(numberSegments[9]));
    }
    else
    {
        //Number too large, show error

        //Set right digit
        memcpy(displaySegments[RIGHT_DIGIT], numberSegments[10], sizeof(numberSegments[10]));

        //Set left digit
        memcpy(displaySegments[LEFT_DIGIT], numberSegments[10], sizeof(numberSegments[10]));
    }

    //Set auxWorld dot if on
    if(auxWorld)
    {
        displaySegments[RIGHT_DIGIT][SEG_H] = ON;
    }
    
    //TODO:
    // Malloc a copy of array and put in queue for display updater
}