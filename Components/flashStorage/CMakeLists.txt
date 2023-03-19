idf_component_register(SRCS "flashStorage.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES log nvs_flash) #https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/build-system.html#example-of-component-requirements