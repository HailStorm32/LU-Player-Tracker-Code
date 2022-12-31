idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES mqttControl wifiControl log) #https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/build-system.html#example-of-component-requirements