idf_component_register(SRCS "iPixel.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_common PRIV_REQUIRES driver) #https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/build-system.html#example-of-component-requirements