#include "gpioControl.h"

//Create a bit masks to select the GPIO pins we want
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<GPIO_TO_MUX_A0) | (1ULL<<GPIO_TO_MUX_A1) | (1ULL<<GPIO_TO_MUX_A2) | (1ULL<<GPIO_DIG_SEL))
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_MODE_BTN)

int initGPIO()
{
    gpio_config_t ioConf = {}; 

    //Configure GPIO ouput
    
    //Disable interrupt
    ioConf.intr_type = GPIO_INTR_DISABLE;
    //Set as output mode
    ioConf.mode = GPIO_MODE_OUTPUT;
    //Bit mask of the pins we want. 64 bit space with each bit representing a GPIO pin, LSB == GPIO 0
    ioConf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //Disable pull-down mode
    ioConf.pull_down_en = 0;
    //Disable pull-up mode
    ioConf.pull_up_en = 0;
    //Configure GPIO with the given settings
    gpio_config(&ioConf);


    //Configure GPIO input

    //Interrupt of negative edge
    ioConf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the pins, use GPIO4/5 here
    ioConf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //Set as input mode
    ioConf.mode = GPIO_MODE_INPUT;
    //Enable pull-up mode
    ioConf.pull_up_en = 1;
    //Configure GPIO with the given settings
    gpio_config(&ioConf);

    //TODO:
    // More setup for mode button
    // Maybe https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-reference/peripherals/pcnt.html for mode selection?

    return 0;
}