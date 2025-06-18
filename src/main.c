#include "wifi.h"
#include "button.h"
#include "water-level.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_attr.h>
#include <driver/gpio.h>
#include <driver/gptimer.h>
#include <esp_task_wdt.h>
#include <esp_sntp.h>
#include <time.h>

#define INTERVAL 10000;

static bool timer_on_alarm(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    (void)edata;
    (void)timer;
    (void)user_data;

    button_update_state();
    return true;
}

void app_main()
{
    // Initialize a button on GPIO4 to be able to call reset_wifi(), it clears the NVS.
    assert(button_init(GPIO_NUM_4));

    // Initialize a water level sensor on GPIO5.
    assert(water_level_init(GPIO_NUM_5));

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
        usleep(50000); // Sleep for 50 milliseconds
        if (BUTTON_FALLING_EDGE == button_get_state())
        {
            wifi_reset();
        }
    }

    time_t now = 0;
    struct tm timeinfo = {0};
    const int retry_count = 10;
    int retry = 0;
    int ms = 1000;

    // Initialize SNTP
    ESP_LOGI("NTP", "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_set_sync_interval(3600 * ms); // Set sync interval to 1 hour
    esp_sntp_setservername(0, "pool.ntp.org");
    setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
    tzset();
    esp_sntp_init();

    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count)
    {
        printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    int i = 0;
    int count = 10;

    while (1)
    {
        time(&now);
        localtime_r(&now, &timeinfo);
        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

        if (i % count == 0)
        {
            printf("Wifi state: %s\n", wifi_connected() ? "Connected" : "Disconnected");
            printf("The current date and time: %s\n", strftime_buf);
            i = 0;
        }

        if (BUTTON_FALLING_EDGE == button_get_state())
        {
            wifi_reset();
        }

        printf("Water level ");
        int state = water_level_get_state();
        if (state == WATER_LEVEL_UNINITIALIZED)
        {
            printf("sensor not initialized.\n");
        }
        else if (state == WATER_LEVEL_HIGH)
        {
            printf("is high (closed).\n");
        }
        else if (state == WATER_LEVEL_LOW)
        {
            printf("is low (open).\n");
        }

        usleep(1000000); // Sleep for 1 second
        ++i;
    }
}