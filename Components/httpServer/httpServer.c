//Kudos to ChatGPT for some of this code. 

#include "esp_http_server.h"
#include "esp_log.h"
//#include "esp_system.h"
//#include "esp_wifi.h"
#include "nvs_flash.h"
#include "httpServer.h"
#include "string.h"
//#include "nvs.h"
#include "flashStorage.h"
#include "htmlFill.h"
#include <math.h>
#include <ctype.h>


/* Constants */
#define TAG                         "static_page"
#define HTML_CONTENT_TYPE           "text/html"
#define MAX_HTTP_RECV_BUFFER        1024 //512
#define RESP_BUFFER                 ( (WIFI_SSID_MAX_LEN * 2) +  (WIFI_PASS_MAX_LEN * 2) )
#define ORIGIN_PAGE_LEN             20
#define DATA_TYPE_LEN               20
#define ACTION_LEN                  30

#define WIFI_ENCODED_SSID_MAX_LEN   (WIFI_SSID_MAX_LEN * 2)
#define WIFI_ENCODED_PASS_MAX_LEN   (WIFI_PASS_MAX_LEN * 2)
#define MQTT_ENCODED_ADDR_MAX_LEN   (MQTT_ADDR_MAX_LEN * 2)
#define MQTT_ENCODED_UNAME_MAX_LEN  (MQTT_UNAME_MAX_LEN * 2)
#define MQTT_ENCODED_PASS_MAX_LEN   (MQTT_PASS_MAX_LEN * 2)


/* Get pointers to embedded HTML pages */
extern const uint8_t root_html_tmpl_start[] asm("_binary_root_html_start");
extern const uint8_t root_html_tmpl_end[]   asm("_binary_root_html_end");

extern const uint8_t wifi_settings_html_tmpl_start[] asm("_binary_wifi_settings_html_start");
extern const uint8_t wifi_settings_html_tmpl_end[]   asm("_binary_wifi_settings_html_end");

extern const uint8_t saved_page_html_tmpl_start[] asm("_binary_saved_page_html_start");
extern const uint8_t saved_page_html_tmpl_end[]   asm("_binary_saved_page_html_end");

extern const uint8_t mqtt_settings_html_tmpl_start[] asm("_binary_mqtt_settings_html_start");
extern const uint8_t mqtt_settings_html_tmpl_end[]   asm("_binary_mqtt_settings_html_end");

extern const uint8_t led_settings_html_tmpl_start[] asm("_binary_led_settings_html_start");
extern const uint8_t led_settings_html_tmpl_end[]   asm("_binary_led_settings_html_end");



char *root_html_response = NULL;
char *wifi_html_response = NULL;
char *saved_html_response = NULL;
char *mqtt_html_response = NULL;
char *led_html_response = NULL;


/* Function prototypes */
static esp_err_t root_handler(httpd_req_t *req);
static esp_err_t save_handler(httpd_req_t *req);
static esp_err_t wifi_handler(httpd_req_t *req);
static esp_err_t mqtt_handler(httpd_req_t *req);
static esp_err_t led_handler(httpd_req_t *req);
static void urlDecode(char *dst, const char *src);

/* URI handlers */
static const httpd_uri_t root_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t wifi_uri = {
    .uri       = "/wifi-settings",
    .method    = HTTP_GET,
    .handler   = wifi_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t save_uri = {
    .uri       = "/save",
    .method    = HTTP_POST,
    .handler   = save_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t mqtt_uri = {
    .uri       = "/mqtt-settings",
    .method    = HTTP_GET,
    .handler   = mqtt_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t led_uri = {
    .uri       = "/led-settings",
    .method    = HTTP_GET,
    .handler   = led_handler,
    .user_ctx  = NULL
};

/* HTTP server config */
static httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();


void initHttpServer()
{
    root_html_response = malloc((root_html_tmpl_end - root_html_tmpl_start));
    saved_html_response = malloc((saved_page_html_tmpl_end - saved_page_html_tmpl_start));
    led_html_response = malloc((led_settings_html_tmpl_end - led_settings_html_tmpl_start));

    if (root_html_response == NULL || saved_html_response == NULL || led_html_response == NULL)
    {
        ESP_LOGE(TAG, "Unable to allocate memory for html responses");
        return;
    }

    memcpy(root_html_response, root_html_tmpl_start, (root_html_tmpl_end - root_html_tmpl_start));
    memcpy(saved_html_response, saved_page_html_tmpl_start, (saved_page_html_tmpl_end - saved_page_html_tmpl_start));
    memcpy(led_html_response, led_settings_html_tmpl_start, (led_settings_html_tmpl_end - led_settings_html_tmpl_start));
    

    // ESP_LOG_BUFFER_HEXDUMP(TAG, root_html_response, strlen(root_html_response)+8, ESP_LOG_DEBUG);
    // ESP_LOGI(TAG, "Starting Addr: 0x%x", (int)root_html_tmpl_start);
    // ESP_LOGI(TAG, "End Addr: 0x%x", (int)root_html_tmpl_end);
    // ESP_LOGI(TAG, "Size: %d bytes", (root_html_tmpl_end - root_html_tmpl_start) );
    // ESP_LOGI(TAG, "LEn Size: %d bytes", strlen(root_html_response) );


    // Create HTTP server instance
    httpd_handle_t server = NULL;
    httpd_config.server_port = 80; // Port 80 for HTTP
    ESP_LOGI(TAG, "Starting HTTP server on port: '%d'", httpd_config.server_port);
    if (httpd_start(&server, &httpd_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return;
    }
    
    // Register URI handlers
    httpd_register_uri_handler(server, &root_uri);
    httpd_register_uri_handler(server, &wifi_uri);
    httpd_register_uri_handler(server, &save_uri);
    httpd_register_uri_handler(server, &mqtt_uri);
    httpd_register_uri_handler(server, &led_uri);
}

/* Root URI handler */
static esp_err_t root_handler(httpd_req_t *req)
{
    // Prepare HTML response
    httpd_resp_set_type(req, HTML_CONTENT_TYPE);
    httpd_resp_send(req, root_html_response, strlen(root_html_response));
    return ESP_OK;
}

/* Wi-Fi URI handler */
static esp_err_t wifi_handler(httpd_req_t *req)
{
    // Allocate memory for the HTML template response
    wifi_html_response = malloc((wifi_settings_html_tmpl_end - wifi_settings_html_tmpl_start));
    if (wifi_html_response == NULL)
    {
        ESP_LOGE(TAG, "Unable to allocate memory for wifi html response");
        return ESP_FAIL;
    }
    memcpy(wifi_html_response, wifi_settings_html_tmpl_start, (wifi_settings_html_tmpl_end - wifi_settings_html_tmpl_start));

    // Fill in and replace the keys in the HTML template
    fillWifiHtmlTmpl(&wifi_html_response);

    // Prepare HTML response
    httpd_resp_set_type(req, HTML_CONTENT_TYPE);
    httpd_resp_send(req, wifi_html_response, strlen(wifi_html_response));

    free(wifi_html_response);

    return ESP_OK;
}

/* MQTT URI handler */
static esp_err_t mqtt_handler(httpd_req_t *req)
{
    // Allocate memory for the HTML template response
    mqtt_html_response = malloc((mqtt_settings_html_tmpl_end - mqtt_settings_html_tmpl_start));
    if (mqtt_html_response == NULL)
    {
        ESP_LOGE(TAG, "Unable to allocate memory for mqtt html response");
        return ESP_FAIL;
    }
    memcpy(mqtt_html_response, mqtt_settings_html_tmpl_start, (mqtt_settings_html_tmpl_end - mqtt_settings_html_tmpl_start));

    // Fill in and replace the keys in the HTML template
    fillMqttHtmlTmpl(&mqtt_html_response);

    // Prepare HTML response
    httpd_resp_set_type(req, HTML_CONTENT_TYPE);
    httpd_resp_send(req, mqtt_html_response, strlen(mqtt_html_response));

    free(mqtt_html_response);

    return ESP_OK;
}

/* LED URI handler */
static esp_err_t led_handler(httpd_req_t *req)
{
    // Prepare HTML response
    httpd_resp_set_type(req, HTML_CONTENT_TYPE);
    httpd_resp_send(req, led_html_response, strlen(led_html_response));
    return ESP_OK;
}

/* Save URI handler */
static esp_err_t save_handler(httpd_req_t *req)
{ 
    char oringinPage[ORIGIN_PAGE_LEN];
    char dataType[DATA_TYPE_LEN];
    char action[ACTION_LEN];

    memset(oringinPage, '\0', ORIGIN_PAGE_LEN);
    memset(dataType, '\0', DATA_TYPE_LEN);
    memset(action, '\0', ACTION_LEN);
    
    if (strcmp(req->uri, "/") == 0)
    {
        httpd_resp_send(req, root_html_response, strlen(root_html_response));
    }
    else if (strcmp(req->uri, "/save") == 0)
    {
        char buf[RESP_BUFFER];
        memset(buf, '\0', RESP_BUFFER);

        int ret, remaining = req->content_len;
        while (remaining > 0)
        {
            ret = httpd_req_recv(req, buf, fmin(remaining, sizeof(buf)));
            if (ret <= 0)
            {
                if (ret == HTTPD_SOCK_ERR_TIMEOUT)
                {
                    httpd_resp_send_408(req);
                }
                return ESP_FAIL;
            }
            remaining -= ret;
        }

        ESP_LOG_BUFFER_HEXDUMP(TAG, buf, RESP_BUFFER, ESP_LOG_DEBUG);

        // Get the origin page
        if (httpd_query_key_value(buf, "origin_page", oringinPage, ORIGIN_PAGE_LEN) != ESP_OK)
        {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to retrieve origin");
            return ESP_FAIL;
        }

        // Get the data type
        if (httpd_query_key_value(buf, "data_type", dataType, DATA_TYPE_LEN) != ESP_OK)
        {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to retrieve data type");
            return ESP_FAIL;
        }

        // Pick the correct response page

        if (strcmp(oringinPage, "wifi-settings") == 0)
        {
            if(strcmp(dataType, "credentials") == 0)
            {
                char decodedSSID[WIFI_SSID_MAX_LEN];
                char decodedPass[WIFI_PASS_MAX_LEN];
                char encodedSSID[WIFI_ENCODED_SSID_MAX_LEN];
                char encodedPassword[WIFI_ENCODED_PASS_MAX_LEN];

                memset(encodedSSID, '\0', WIFI_ENCODED_SSID_MAX_LEN);
                memset(encodedPassword, '\0', WIFI_ENCODED_PASS_MAX_LEN);
                memset(decodedSSID, '\0', WIFI_SSID_MAX_LEN);
                memset(decodedPass, '\0', WIFI_PASS_MAX_LEN);

                // Get the SSID and password
                if (httpd_query_key_value(buf, "ssid", encodedSSID, WIFI_ENCODED_SSID_MAX_LEN) != ESP_OK ||
                    httpd_query_key_value(buf, "password", encodedPassword, WIFI_ENCODED_PASS_MAX_LEN) != ESP_OK)
                {
                    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameters");
                    return ESP_FAIL;
                }

                ESP_LOGI(TAG, "Got (before decode):\nSSID: %s\nPASS: %s", encodedSSID, encodedPassword);

                //Decode the URLdecoded credentials
                urlDecode(decodedSSID, encodedSSID);
                urlDecode(decodedPass, encodedPassword);

                ESP_LOGI(TAG, "Got (after decode):\nSSID: %s\nPASS: %s", decodedSSID, decodedPass);

                //Store the credentials
                if(storeWifiCredentials(decodedSSID, decodedPass) != ESP_OK)
                {
                    ESP_LOGE(TAG, "Unable to store ssid and/or password");
                    return ESP_FAIL;
                }

                httpd_resp_send(req, saved_html_response, strlen(saved_html_response));
            }
            else
            {
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid data type");
                return ESP_FAIL;
            }
            
        }
        else if (strcmp(oringinPage, "mqtt-settings") == 0)
        {
            if (strcmp(dataType, "credentials") == 0)
            {
                mqttSettings_t mqttSettings;
                char encodedAddress[MQTT_ENCODED_ADDR_MAX_LEN];
                char encodedUsername[MQTT_ENCODED_UNAME_MAX_LEN];
                char encodedPassword[MQTT_ENCODED_PASS_MAX_LEN];

                memset(encodedAddress, '\0', MQTT_ENCODED_ADDR_MAX_LEN);
                memset(encodedUsername, '\0', MQTT_ENCODED_UNAME_MAX_LEN);
                memset(encodedPassword, '\0', MQTT_ENCODED_PASS_MAX_LEN);
                memset(&mqttSettings, 0, sizeof(mqttSettings_t));

                // Get the action
                if (httpd_query_key_value(buf, "action", action, ACTION_LEN) != ESP_OK)
                {
                    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to retrieve action");
                    return ESP_FAIL;
                }

                if (strcmp(action, "save_address") == 0)
                {
                    // Get the address
                    if (httpd_query_key_value(buf, "address", encodedAddress, MQTT_ENCODED_ADDR_MAX_LEN) != ESP_OK)
                    {
                        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameters");
                        return ESP_FAIL;
                    }

                    //Decode the URLencoded credentials
                    urlDecode(mqttSettings.address, encodedAddress);

                    //Store the credentials
                    if(storeMqttSettings(&mqttSettings) != ESP_OK)
                    {
                        ESP_LOGE(TAG, "Unable to store mqtt address");
                        return ESP_FAIL;
                    }
                }
                else if (strcmp(action, "save_credentials") == 0)
                {
                    // Get the username and password
                    if (httpd_query_key_value(buf, "username", encodedUsername, MQTT_ENCODED_UNAME_MAX_LEN) != ESP_OK ||
                        httpd_query_key_value(buf, "password", encodedPassword, MQTT_ENCODED_PASS_MAX_LEN) != ESP_OK)
                    {
                        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameters");
                        return ESP_FAIL;
                    }

                    //Decode the URLencoded credentials
                    urlDecode(mqttSettings.username, encodedUsername);
                    urlDecode(mqttSettings.password, encodedPassword);

                    //Store the credentials
                    if(storeMqttSettings(&mqttSettings) != ESP_OK)
                    {
                        ESP_LOGE(TAG, "Unable to store mqtt credentials");
                        return ESP_FAIL;
                    }
                }
                else if(strcmp(action, "save_all") == 0)
                {
                    // Get the address
                    if (httpd_query_key_value(buf, "address", encodedAddress, MQTT_ENCODED_ADDR_MAX_LEN) != ESP_OK)
                    {
                        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameters");
                        return ESP_FAIL;
                    }

                    // Get the username and password
                    if (httpd_query_key_value(buf, "username", encodedUsername, MQTT_ENCODED_UNAME_MAX_LEN) != ESP_OK ||
                        httpd_query_key_value(buf, "password", encodedPassword, MQTT_ENCODED_PASS_MAX_LEN) != ESP_OK)
                    {
                        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameters");
                        return ESP_FAIL;
                    }

                    //Decode the URLencoded credentials
                    urlDecode(mqttSettings.address, encodedAddress);
                    urlDecode(mqttSettings.username, encodedUsername);
                    urlDecode(mqttSettings.password, encodedPassword);

                    //Store the credentials
                    if(storeMqttSettings(&mqttSettings) != ESP_OK)
                    {
                        ESP_LOGE(TAG, "Unable to store mqtt credentials");
                        return ESP_FAIL;
                    }
                }
                else
                {
                    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid action");
                    return ESP_FAIL;
                }
            }

            httpd_resp_send(req, saved_html_response, strlen(saved_html_response));
        }
        else if (strcmp(oringinPage, "led-settings") == 0)
        {
            //TODO: Process LED settings

            httpd_resp_send(req, saved_html_response, strlen(saved_html_response));
        }
        else
        {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid origin page");
            return ESP_FAIL;
        }
    }

    return ESP_OK;
}

static void urlDecode(char *dst, const char *src) 
{
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a')
                a -= 'a'-'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';
            if (b >= 'a')
                b -= 'a'-'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            *dst++ = 16*a+b;
            src+=3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}
