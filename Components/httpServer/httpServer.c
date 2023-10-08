//Kudos to ChatGPT for this code. 

#include "esp_http_server.h"
#include "esp_log.h"
//#include "esp_system.h"
//#include "esp_wifi.h"
#include "nvs_flash.h"
#include "httpServer.h"
#include "string.h"


/* Constants */
#define TAG "static_page"
#define HTML_CONTENT_TYPE "text/html"
#define MAX_HTTP_RECV_BUFFER 1024 //512

/* Function prototypes */
static esp_err_t root_handler(httpd_req_t *req);
static esp_err_t save_handler(httpd_req_t *req);

/* URI handlers */
static const httpd_uri_t root_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_handler,
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
    // Initialize NVS flash memory
    //ESP_ERROR_CHECK(nvs_flash_init());
    
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
    httpd_register_uri_handler(server, &save_uri);
}

/* Root URI handler */
static esp_err_t root_handler(httpd_req_t *req)
{
    char val[30];
    // Prepare HTML response
    const char *html_response = "<html><head><title>Wi-Fi Credentials</title></head><body><form method='post' action='/save'>SSID: <input type='text' name='ssid'><br>Password: <input type='text' name='password'><br><br><input type='submit' value='Save'></form></body></html>";
    httpd_resp_set_type(req, HTML_CONTENT_TYPE);
    httpd_resp_send(req, html_response, strlen(html_response));
    return ESP_OK;
}

/* Save URI handler */
static esp_err_t save_handler(httpd_req_t *req)
{
    // TODO: Implement function to save Wi-Fi credentials to flash
    
    // Send HTTP response
    const char *html_response = "<html><head><title>Saved</title></head><body>Wi-Fi credentials saved.</body></html>";
    httpd_resp_set_type(req, HTML_CONTENT_TYPE);
    httpd_resp_send(req, html_response, strlen(html_response));
    return ESP_OK;
}
