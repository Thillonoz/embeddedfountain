#ifndef MQTT_H
#define MQTT_H

// You need to create your own mqttcredentials.h file and define your own MQTT_USERNAME, MQTT_PASSWORD and MQTT_SERVER.
// #include "mqttcredentials.h"
// -------------------------

#include <mqtt_client.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

int mqtt_pump_state{0};

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

static void mqtt_init(void);

bool mqtt_connected(void);

void mqtt_publish(void);

#endif // MQTT_H