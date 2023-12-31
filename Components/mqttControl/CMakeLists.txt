idf_component_register(SRCS "mqtt.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_wifi mqtt esp_event log linkedList ledControl flashStorage nvs_flash) #https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/build-system.html#example-of-component-requirements