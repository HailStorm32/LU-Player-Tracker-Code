#pragma once

#include "driver/gpio.h"

#define GPIO_TO_MUX_A0  4
#define GPIO_TO_MUX_A1  5
#define GPIO_TO_MUX_A2  6
#define GPIO_DIG_SEL    7
#define GPIO_MODE_BTN   12

#define LOW             0
#define HIGH            1

#define RIGHT_DIGIT     LOW
#define LEFT_DIGIT      HIGH

int initGPIO();