idf_component_register(SRCS "httpServer.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES log nvs_flash esp_http_server flashStorage
                    EMBED_TXTFILES HTML/root.html HTML/wifi-settings.html HTML/saved_page.html HTML/mqtt_settings.html HTML/led_settings.html ) #https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/build-system.html#example-of-component-requirements
                                                         #https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-guides/build-system.html?highlight=target_add_binary_data#embedding-binary-data