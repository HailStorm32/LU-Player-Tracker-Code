#ifndef LED_CONTROL
#define LED_CONTROL

typedef struct msgInfo
{
    char* msgPtr;
    int msgSize;
} msgInfo_t, *msgInfoPtr_t;

int initLedControl();

#endif