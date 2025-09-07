#include "pins.h"
#include "mqtt.h"
#include "bsp.h"
#include <esp_log.h>
#include <driver/gpio.h>

static volatile bool connected{false};

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        connected = true;

        msg_id = esp_mqtt_client_subscribe_single(client, TOPIC_PUMP_CONTROL, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        connected = false;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        if (0 == strncmp(event->topic, TOPIC_PUMP_CONTROL, event->topic_len))
        {
            event->data[event->data_len] = 0;
            if (0 == strcmp("on", event->data))
            {
                mqtt_pump_state = 1;
                bsp_pin_write(PUMP_CONTROL_PIN, mqtt_pump_state);
            }
            else if (0 == strcmp("off", event->data))
            {
                mqtt_pump_state = 0;
                bsp_pin_write(PUMP_CONTROL_PIN, mqtt_pump_state);
            }
            else
            {
                printf("Invalid pump state\r\n");
            }
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void init_mqtt(void)
{
    esp_mqtt_client_config_t config;
    memset(&config, 0, sizeof(config));
    config.broker.address.port = MQTT_PORT;
    config.broker.address.uri = MQTT_SERVER;
    config.broker.verification.certificate = (const char *)cert;
    config.credentials.authentication.password = MQTT_PASSWORD;
    config.credentials.client_id = MQTT_CLIENTID;
    config.credentials.username = MQTT_USERNAME;

    client = esp_mqtt_client_init(&config);
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
}

bool mqtt_connected(void)
{
    return connected;
}