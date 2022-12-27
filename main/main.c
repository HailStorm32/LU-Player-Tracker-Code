#include <stdio.h>
#include "mqtt.h"
#include "wifi.h"

int app_main(void)
{
    InitWifiSta();
    initMqttClient();

    printf("This is a test\n\n This is a Tesf\n\n");
    int test = 10;
    printf("%d", test);
    return 0;
}