//Kudos to ChatGPT for this code. 

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


/* Constants */
#define TAG                     "static_page"
#define HTML_CONTENT_TYPE       "text/html"
#define MAX_HTTP_RECV_BUFFER    1024 //512
#define SSID_MAX_LEN            32
#define PASS_MAX_LEN            64

const char *html_response = "<form method=\"post\" action=\"/save\">   <label for=\"ssid\">SSID:</label>   <input type=\"text\" id=\"ssid\" name=\"ssid\" maxlength=\"32\">   <span id=\"ssid-error\" style=\"color: red; display: none;\">Maximum length exceeded</span><br>    <label for=\"password\">Password:</label>   <input type=\"text\" id=\"password\" name=\"password\" maxlength=\"64\">   <span id=\"password-error\" style=\"color: red; display: none;\">Maximum length exceeded</span><br>    <input type=\"submit\" value=\"Save\"> </form>  <style>    .input-error {     display: none;     color: red;   }     input:invalid + .input-error {     display: inline;   } </style>  <script>    const ssidInput = document.getElementById(\"ssid\");   const passwordInput = document.getElementById(\"password\");    ssidInput.addEventListener(\"input\", validateInput);   passwordInput.addEventListener(\"input\", validateInput);     function validateInput(event) {     const input = event.target;     const error = document.getElementById(`${input.id}-error`);      if (input.value.length > input.maxLength) {       input.setCustomValidity(`Maximum length is ${input.maxLength}`);       error.style.display = \"inline\";     } else {       input.setCustomValidity(\"\");       error.style.display = \"none\";     }   } </script>";
//"<html><head><title>Wi-Fi Credentials</title></head><body><form method='post' action='/save'>SSID: <input type='text' maxlength='32' name='ssid'><br>Password: <input type='text' maxlength='64' name='password'><br><br><input type='submit' value='Save'></form></body></html>";

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
    // Prepare HTML response
    httpd_resp_set_type(req, HTML_CONTENT_TYPE);
    httpd_resp_send(req, html_response, strlen(html_response));
    return ESP_OK;
}

/* Save URI handler */
static esp_err_t save_handler(httpd_req_t *req)
{
    // TODO: Implement function to save Wi-Fi credentials to flash

    char* ssid = malloc(SSID_MAX_LEN);
    char* password = malloc(PASS_MAX_LEN);

    if (ssid == NULL || password == NULL)
    {
       ESP_LOGE(TAG, "Unable to allocate memory for ssid and/or password");
    }

    memset(ssid, '\0', SSID_MAX_LEN);
    memset(password, '\0', PASS_MAX_LEN);
    
    if (strcmp(req->uri, "/") == 0)
    {
        httpd_resp_send(req, html_response, strlen(html_response));
    }
    else if (strcmp(req->uri, "/save") == 0)
    {
        char buf[512];
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

        if (httpd_query_key_value(buf, "ssid", ssid, SSID_MAX_LEN) != ESP_OK ||
            httpd_query_key_value(buf, "password", password, PASS_MAX_LEN) != ESP_OK)
        {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameters");
            return ESP_FAIL;
        }
    }

    //Store the credentials
    if(storeWifiCredentials(ssid, password) != ESP_OK)
    {
        ESP_LOGE(TAG, "Unable to store ssid and/or password");
        free(ssid);
        free(password);
        return ESP_FAIL;
    }

    // Send HTTP response
    const char *html_response = "<html><head><title>Saved</title></head><body>Wi-Fi credentials saved.</body></html>";
    httpd_resp_set_type(req, HTML_CONTENT_TYPE);
    httpd_resp_send(req, html_response, strlen(html_response));

    free(ssid);
    free(password);
    return ESP_OK;
}
