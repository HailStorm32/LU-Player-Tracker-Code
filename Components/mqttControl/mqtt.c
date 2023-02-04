#include <stdint.h>
#include <stdio.h>
#include "esp_event.h"
#include "mqtt.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "linkedList.h"
#include "ledControl.h"
#include "../../credentials.h"


esp_mqtt_client_handle_t mqttClient;
nodePtr_t linkedListHead = NULL;

extern QueueHandle_t mqttJsonQueue;

const char* MQTT_TOPIC = "playerStatus";
const char* MQTT_LOG_TAG = "MQTT_Control";

#define NULL_CHARS 1

typedef struct msgData
{
	char* topic;
	char* msg;
	uint16_t msgId;
	int currentLength;
} msgData_t, *msgDataPtr_t;


static void log_error_if_nonzero(const char* message, int error_code)
{
	if (error_code != 0) {
		ESP_LOGE("MQTT", "Last error %s: 0x%x", message, error_code);
	}
}

static void mqtt_event_handler(void* handlerArgs, esp_event_base_t base, int32_t eventId, void* eventData)
{
	ESP_LOGD(MQTT_LOG_TAG, "Event dispatched from event loop base=%s, eventId=%ld", base, eventId);
	esp_mqtt_event_handle_t event = eventData;
	int msgId;

	switch (eventId) //(esp_mqtt_event_id_t)
	{
	case MQTT_EVENT_CONNECTED:
		ESP_LOGI(MQTT_LOG_TAG, "MQTT_EVENT_CONNECTED");

		msgId = esp_mqtt_client_subscribe(mqttClient, MQTT_TOPIC, 1);
		ESP_LOGI(MQTT_LOG_TAG, "sent subscribe successful, msgId=%d", msgId);

		//msgId = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
		//ESP_LOGI(MQTT_LOG_TAG, "sent subscribe successful, msgId=%d", msgId);

		//msgId = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
		//ESP_LOGI(MQTT_LOG_TAG, "sent subscribe successful, msgId=%d", msgId);

		//msgId = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
		//ESP_LOGI(MQTT_LOG_TAG, "sent unsubscribe successful, msgId=%d", msgId);
		break;
	case MQTT_EVENT_DISCONNECTED:
		ESP_LOGI(MQTT_LOG_TAG, "MQTT_EVENT_DISCONNECTED");
		break;

	case MQTT_EVENT_SUBSCRIBED:
		ESP_LOGI(MQTT_LOG_TAG, "MQTT_EVENT_SUBSCRIBED, msgId=%d", event->msg_id);
		//msgId = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
		//ESP_LOGI(MQTT_LOG_TAG, "sent publish successful, msgId=%d", msgId);
		break;
	case MQTT_EVENT_UNSUBSCRIBED:
		ESP_LOGI(MQTT_LOG_TAG, "MQTT_EVENT_UNSUBSCRIBED, msgId=%d", event->msg_id);
		break;
	case MQTT_EVENT_PUBLISHED:
		ESP_LOGI(MQTT_LOG_TAG, "MQTT_EVENT_PUBLISHED, msgId=%d", event->msg_id);
		break;
	case MQTT_EVENT_DATA:
		ESP_LOGI(MQTT_LOG_TAG, "MQTT_EVENT_DATA");

		nodePtr_t resultNode;

		//If MQTT data is split
		if (event->total_data_len > event->data_len)
		{
			ESP_LOGD(MQTT_LOG_TAG, "Message is split: %d %d %d %d", event->msg_id, event->total_data_len , event->data_len, event->topic_len);
			
			//See if we already have part of the message
			resultNode = LL_find(linkedListHead, event->msg_id);

			//If we dont have the part of the message, create an entry
			if (resultNode == NULL)
			{
				ESP_LOGD(MQTT_LOG_TAG, "Creating new LL node");

				msgDataPtr_t msgData = malloc(sizeof(msgData_t));
				
				if(msgData == NULL)
    			{
       				ESP_LOGE(MQTT_LOG_TAG, "ERROR: Unable to allocate memory for MQTT LL data");
        			return;
   				}

				msgData->msgId = event->msg_id;
				msgData->currentLength = event->data_len;
				msgData->topic = malloc(strlen(event->topic) + NULL_CHARS);
				msgData->msg = malloc(strlen(event->data) + NULL_CHARS);

				if(msgData->topic == NULL || msgData->msg == NULL)
    			{
       				ESP_LOGE(MQTT_LOG_TAG, "ERROR: Unable to allocate memory for MQTT LL data-msg and/or data->topic");
        			return;
   				}

				//Copy over the topic and message data
				strcpy(msgData->topic, event->topic);
				strcpy(msgData->msg, event->data);

				LL_insertFirst(&linkedListHead, event->msg_id, msgData);
			}
			else   //strncmp(((msgDataPtr_t)resultNode->dataPtr)->topic, event->topic, event->topic_len) == 0 //Topic isnt sent in subsequent messages
			{
				ESP_LOGD(MQTT_LOG_TAG, "Found message in LL");

				msgDataPtr_t msgData = (msgDataPtr_t)resultNode->dataPtr;

				//See if we have the full message, else add on to the nodeData
				if (msgData->currentLength + event->data_len == event->total_data_len)
				{
					ESP_LOGI(MQTT_LOG_TAG, "Have full message");

					//Update length of the message we have currently
					msgData->currentLength += event->data_len;

					//Reallocate space to fit the next part of the message
					msgData->msg = realloc(msgData->msg, (msgData->currentLength + event->data_len) + NULL_CHARS);
					
					if (msgData->msg == NULL)
					{
						ESP_LOGE(MQTT_LOG_TAG, "ERROR: Unable to reallocate memory for MQTT LL msg data");
        				return;		
					}

					//Append the new message to the what we curently had
					strncat(msgData->msg, event->data, event->data_len);

					ESP_LOGD(MQTT_LOG_TAG, "Data:\n %s\n", msgData->msg);

					//Create pointers that will hold the struct and msg copy
					msgInfoPtr_t msgQueueStruct = malloc(sizeof(msgInfo_t));
					char* msgCpy = malloc(msgData->currentLength + NULL_CHARS);

					if(msgQueueStruct == NULL || msgCpy == NULL)
					{
						ESP_LOGE(MQTT_LOG_TAG, "ERROR: Unable to allocate memory for MQTT msg struct and/or msgCpy");
					}

					//Copy over the message b/c we will be freeing up the current pointer
					memcpy(msgCpy, msgData->msg, msgData->currentLength + NULL_CHARS);

					msgQueueStruct->msgPtr = msgCpy;
					msgQueueStruct->msgSize = msgData->currentLength;
					
					//Queue and send to LED control
					if(xQueueSendToBack(mqttJsonQueue, &msgQueueStruct, 10) == errQUEUE_FULL)
					{
						ESP_LOGE(MQTT_LOG_TAG, "ERROR: mqttJsonQueue is full");
					}
					
					//Free up the space used by the msgData struct
					free(msgData->msg);
					free(msgData->topic);
					//free(msgData);

					//Make sure to remove the LL entry
					LL_removeItem(&linkedListHead, event->msg_id);
				}
				else
				{
					ESP_LOGI(MQTT_LOG_TAG, "Have some message");
					
					//Update length of the message we have currently
					msgData->currentLength += event->data_len;

					//Reallocate space to fit the next part of the message
					msgData->msg = realloc(msgData->msg, msgData->currentLength + NULL_CHARS);

					if (msgData->msg == NULL)
					{
						ESP_LOGE(MQTT_LOG_TAG, "ERROR: Unable to reallocate memory for LL msg data");
        				return;		
					}
					
					//Append the new message to the what we curently had
					strncat(msgData->msg, event->data, event->data_len);
				}

			}
		}
		else
		{
			ESP_LOGD(MQTT_LOG_TAG, "dataPtr=\n%.*s\r\n", event->data_len, event->data);
			
			//Create pointers that will hold the struct and msg copy
			msgInfoPtr_t msgQueueStruct = malloc(sizeof(msgInfo_t));
			char* msgCpy = malloc(event->data_len + NULL_CHARS);

			if(msgQueueStruct == NULL || msgCpy == NULL)
			{
				ESP_LOGE(MQTT_LOG_TAG, "ERROR: Unable to allocate memory for MQTT msg struct and/or msgCpy");
			}

			//Copy over the message b/c we will be freeing up the current pointer
			memcpy(msgCpy, event->data, event->data_len);

			msgQueueStruct->msgPtr = msgCpy;
			msgQueueStruct->msgSize = event->data_len;

			//Queue and send to LED control
			if(xQueueSendToBack(mqttJsonQueue, &msgQueueStruct, 10) == errQUEUE_FULL)
			{
				ESP_LOGE(MQTT_LOG_TAG, "ERROR: mqttJsonQueue is full");
			}
		}

/* 		printf("QoS: %d\n", event->qos);
		printf("MSGID: %d\n", event->msg_id);
		printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
		printf("dataPtr=%.*s\r\n", event->data_len, event->data); */
		break;
	case MQTT_EVENT_ERROR:
		ESP_LOGI(MQTT_LOG_TAG, "MQTT_EVENT_ERROR");
		if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
		{
			log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
			log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
			log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
			ESP_LOGI(MQTT_LOG_TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
		}
		break;
	default:
		ESP_LOGI(MQTT_LOG_TAG, "Other event id:%d", event->event_id);
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