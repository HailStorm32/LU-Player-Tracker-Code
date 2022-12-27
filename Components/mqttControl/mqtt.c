#include <stdint.h>
#include <stdio.h>
#include "esp_event.h"
#include "mqtt.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "linkedList.h"
#include "../../credentials.h"

esp_mqtt_client_handle_t mqttClient;

const char* MQTT_TOPIC = "playerStatus";


static void log_error_if_nonzero(const char* message, int error_code)
{
	if (error_code != 0) {
		ESP_LOGE("MQTT", "Last error %s: 0x%x", message, error_code);
	}
}

static void mqtt_event_handler(void* handlerArgs, esp_event_base_t base, int32_t eventId, void* eventData)
{
	ESP_LOGD("MQTT", "Event dispatched from event loop base=%s, eventId=%ld", base, eventId);
	esp_mqtt_event_handle_t event = eventData;
	esp_mqtt_client_handle_t client = event->client;
	int msgId;
	uint8_t msgNum = 0;

	switch (eventId) //(esp_mqtt_event_id_t)
	{
	case MQTT_EVENT_CONNECTED:
		ESP_LOGI("MQTT", "MQTT_EVENT_CONNECTED");

		msgId = esp_mqtt_client_subscribe(mqttClient, MQTT_TOPIC, 0);
		ESP_LOGI("MQTT", "sent subscribe successful, msgId=%d", msgId);

		//msgId = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
		//ESP_LOGI("MQTT", "sent subscribe successful, msgId=%d", msgId);

		//msgId = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
		//ESP_LOGI("MQTT", "sent subscribe successful, msgId=%d", msgId);

		//msgId = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
		//ESP_LOGI("MQTT", "sent unsubscribe successful, msgId=%d", msgId);
		break;
	case MQTT_EVENT_DISCONNECTED:
		ESP_LOGI("MQTT", "MQTT_EVENT_DISCONNECTED");
		break;

	case MQTT_EVENT_SUBSCRIBED:
		ESP_LOGI("MQTT", "MQTT_EVENT_SUBSCRIBED, msgId=%d", event->msg_id);
		//msgId = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
		//ESP_LOGI("MQTT", "sent publish successful, msgId=%d", msgId);
		break;
	case MQTT_EVENT_UNSUBSCRIBED:
		ESP_LOGI("MQTT", "MQTT_EVENT_UNSUBSCRIBED, msgId=%d", event->msg_id);
		break;
	case MQTT_EVENT_PUBLISHED:
		ESP_LOGI("MQTT", "MQTT_EVENT_PUBLISHED, msgId=%d", event->msg_id);
		break;
	case MQTT_EVENT_DATA:
		ESP_LOGI("MQTT", "MQTT_EVENT_DATA");

		//TODO: 
		// Pass payload to JSON parser
        // Pass parsed JSON to LED updater

		// if(event->topic == MQTT_TOPIC && msgNum == 0)
		// {
			
		// }
		printf("MSGID:%d\n", event->msg_id);
		printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
		printf("DATA=%.*s\r\n", event->data_len, event->data);
		break;
	case MQTT_EVENT_ERROR:
		ESP_LOGI("MQTT", "MQTT_EVENT_ERROR");
		if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
		{
			log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
			log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
			log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
			ESP_LOGI("MQTT", "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
		}
		break;
	default:
		ESP_LOGI("MQTT", "Other event id:%d", event->event_id);
		break;
	}
}

void initMqttClient(void)
{
	const esp_mqtt_client_config_t mqtt_cfg = {
		.broker.address.uri = BROKER_ADDR,
		.credentials = {
            .username = BROKER_UNAME,
			.authentication = {
				.password = BROKER_PASS
			},
		}
	};

	mqttClient = esp_mqtt_client_init(&mqtt_cfg); //Initalize and create client
	esp_mqtt_client_register_event(mqttClient, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL); //Create an event loop to listen for any mqtt event 
	esp_mqtt_client_start(mqttClient); //Start the client
}