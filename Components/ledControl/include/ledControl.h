#ifndef LED_CONTROL
#define LED_CONTROL

#define NUM_OF_LEDS 15

typedef struct msgInfo
{
    char* msgPtr;
    int msgSize;
} msgInfo_t, *msgInfoPtr_t;

typedef struct worldLed
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t ledArrayIndexStart;
    float brightness;
} worldLed_t;

enum WRLD_INDEX {VE, AG, PC, GF, FV, NS, LC, NT, NINJA, CP, PORT, MOON, ROBOT, DEEP, STAR};

int initLedControl();

void ledUpdateTask();

void setLedColorSingle(const uint8_t worldIndex, const uint8_t red, const uint8_t green, const uint8_t blue, const float brightness);

void setLedColorAll(const worldLed_t* worldLedStructArray);

void getLedColorAll(worldLed_t* worldLedStructArray);

#endif