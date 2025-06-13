#include "wifi.h"
#include "button.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_attr.h>
#include <driver/gpio.h>
#include <driver/gptimer.h>
#include <esp_task_wdt.h>
#include <esp_http_client.h>

#define INTERVAL 10000;

#define SERVER_URL "http://numbersapi.com/"

static bool timer_on_alarm(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    (void)edata;
    (void)timer;
    (void)user_data;

    button_update_state();
    return true;
}

void connect_wifi(void)
{
    if (ESP_OK == wifi_init())
    {
        printf("Wifi initialized successfully.\n");
    }
    else
    {
        printf("Failed to initialize wifi.\n");
    }
}

void app_main()
{
    /* Initialize the button on GPIO4
    ** and enable the internal pull-up resistor. */
    assert(button_init(GPIO_NUM_4));

    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config;
    memset(&timer_config, 0, sizeof(timer_config));
    timer_config.resolution_hz = 1000000;
    timer_config.direction = GPTIMER_COUNT_UP;
    timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs;
    cbs.on_alarm = timer_on_alarm;
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    gptimer_alarm_config_t alarm_config;
    alarm_config.reload_count = 0;
    alarm_config.alarm_count = INTERVAL;
    alarm_config.flags.auto_reload_on_alarm = true;
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    // Initialize the wifi and connect to the network.
    ESP_ERROR_CHECK(wifi_init());

    while (!wifi_connected())
    {
        putchar('.');
        usleep(50000);

        if (BUTTON_FALLING_EDGE == button_get_state())
        {
            wifi_reset();
        }
    }

    while (1)
    {
        if (BUTTON_FALLING_EDGE == button_get_state())
        {
            wifi_reset();
        }
        printf("Wifi state: %s\n", wifi_connected() ? "Connected" : "Disconnected");
        usleep(1000000); // Sleep for 1 second
        if (!wifi_connected())
        {
            printf("Wifi disconnected. Reinitializing...\n");
            wifi_reset();
        }
    }
}