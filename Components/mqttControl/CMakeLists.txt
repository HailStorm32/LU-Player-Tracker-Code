idf_component_register(SRCS "mqtt.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES mqtt esp_event log linkedList) #https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/build-system.html#example-of-component-requirements