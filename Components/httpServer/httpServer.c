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
#include <math.h>
#include <ctype.h>


/* Constants */
#define TAG                     "static_page"
#define HTML_CONTENT_TYPE       "text/html"
#define MAX_HTTP_RECV_BUFFER    1024 //512
#define RESP_BUFFER             ( (SSID_MAX_LEN * 2) +  (PASS_MAX_LEN * 2) )
#define ORIGIN_PAGE_LEN         20
#define DATA_TYPE_LEN           20


/* Get pointers to embedded HTML pages */
extern const uint8_t root_html_start[] asm("_binary_root_html_start");
extern const uint8_t root_html_end[]   asm("_binary_root_html_end");

extern const uint8_t wifi_settings_html_start[] asm("_binary_wifi_settings_html_start");
extern const uint8_t wifi_settings_html_end[]   asm("_binary_wifi_settings_html_end");

extern const uint8_t saved_page_html_start[] asm("_binary_saved_page_html_start");
extern const uint8_t saved_page_html_end[]   asm("_binary_saved_page_html_end");



const char *root_html_response = (char*)root_html_start;
const char *wifi_html_response = (char*)wifi_settings_html_start;
const char *saved_html_response = (char*)saved_page_html_start;


/* Function prototypes */
static esp_err_t root_handler(httpd_req_t *req);
static esp_err_t save_handler(httpd_req_t *req);
static esp_err_t wifi_handler(httpd_req_t *req);
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

/* HTTP server config */
static httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();


void initHttpServer()
{
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
    // Prepare HTML response
    httpd_resp_set_type(req, HTML_CONTENT_TYPE);
    httpd_resp_send(req, wifi_html_response, strlen(wifi_html_response));
    return ESP_OK;
}

/* Save URI handler */
static esp_err_t save_handler(httpd_req_t *req)
{
    char decodedSSID[SSID_MAX_LEN];
    char decodedPass[PASS_MAX_LEN];
    char ssid[SSID_MAX_LEN];
    char password[PASS_MAX_LEN];
    char oringinPage[ORIGIN_PAGE_LEN];
    char dataType[DATA_TYPE_LEN];

    memset(ssid, '\0', SSID_MAX_LEN);
    memset(password, '\0', PASS_MAX_LEN);
    memset(decodedSSID, '\0', SSID_MAX_LEN);
    memset(decodedPass, '\0', PASS_MAX_LEN);
    
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
                // Get the SSID and password
                if (httpd_query_key_value(buf, "ssid", ssid, SSID_MAX_LEN) != ESP_OK ||
                    httpd_query_key_value(buf, "password", password, PASS_MAX_LEN) != ESP_OK)
                {
                    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameters");
                    return ESP_FAIL;
                }

                ESP_LOGI(TAG, "Got (before decode):\nSSID: %s\nPASS: %s", ssid, password);

                //Decode the URLdecoded credentials
                urlDecode(decodedSSID, ssid);
                urlDecode(decodedPass, password);

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
            //TODO: Process MQTT settings

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
