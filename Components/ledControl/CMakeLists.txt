idf_component_register(SRCS "ledControl.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES freertos log cJSON iPixelDriver driver soc sevenSegmentControl) #https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/build-system.html#example-of-component-requirements