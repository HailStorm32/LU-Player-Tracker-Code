#include <stdio.h>
#include "mqtt.h"

int app_main(void)
{
    initMqttClient();

    printf("This is a test\n\n This is a Tesf\n\n");
    int test = 10;
    printf("%d", test);
    return 0;
}