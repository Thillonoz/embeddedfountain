#include "bsp.h"
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
#define OFF 0
#define ON 1

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

    // Initialize an output on GPIO6 for the pump control.
    assert(bsp_pin_config(GPIO_NUM_6, GPIO_MODE_OUTPUT, GPIO_PULLUP_DISABLE));

    // Initialize a timer to call button_update_state() every INTERVAL microseconds.
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config;
    memset(&timer_config, 0, sizeof(timer_config));
    timer_config.resolution_hz = 1000000;
    timer_config.direction = GPTIMER_COUNT_UP;
    timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    // Register the timer callback to be called on alarm events.
    gptimer_event_callbacks_t cbs;
    cbs.on_alarm = timer_on_alarm;
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    // Set the timer to trigger an alarm every INTERVAL microseconds.
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

    // Wait for SNTP to synchronize time
    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count)
    {
        printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    int iteration = 0;        // Counts every while loop, dependant on sleep time
    int count = 30;           // Print status every "count" seconds
    int waterLevelBuffer = 0; // Buffer to prevent rapid state changes
    int pumpState = OFF;
    int desiredPumpState = OFF; // Desired state of the pump, 0 = off, 1 = on

    while (1)
    {
        time(&now);
        localtime_r(&now, &timeinfo);
        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

        // Print status every 'count' iterations
        if (iteration % count == 0)
        {
            printf("Wifi state: %s\n", wifi_connected() ? "Connected" : "Disconnected");
            printf("The current date and time: %s\n", strftime_buf);
            iteration = 0; // Reset iteration counter, prevent overflow
        }

        // Check if the button is pressed to reset the WiFi connection
        if (BUTTON_FALLING_EDGE == button_get_state())
        {
            wifi_reset();
        }

        // Timetable logic for pump control
        if (timeinfo.tm_hour >= 6 && timeinfo.tm_hour < 22)
        {
            printf("Water Level State: %d ", waterLevelBuffer);

            printf("Water level ");

            int waterLevelState = water_level_get_state();
            if (waterLevelState == WATER_LEVEL_UNINITIALIZED)
            {
                printf("sensor not initialized.\n");
            }
            else if (waterLevelState == WATER_LEVEL_HIGH)
            {
                ++waterLevelBuffer;
                if (waterLevelBuffer > 4)
                {
                    desiredPumpState = ON; // Turn on the pump
                    waterLevelBuffer = 4;  // Prevent overflow
                }
                printf("is high (closed).\n");
            }
            else if (waterLevelState == WATER_LEVEL_LOW)
            {
                --waterLevelBuffer;
                if (waterLevelBuffer < 0)
                {
                    desiredPumpState = OFF; // Turn off the pump
                    waterLevelBuffer = 0;   // Prevent negative overflow
                }
                printf("is low (open).\n");
            }
        }
        else
        {
            desiredPumpState = OFF; // Turn off the pump outside of active hours
        }

        // If the desired pump state is different from the current state, change it
        if (desiredPumpState != pumpState)
        {
            bsp_pin_write(GPIO_NUM_6, desiredPumpState ? ON : OFF);
            printf("Pump is turned %s.\n", desiredPumpState ? "on" : "off");
            pumpState = desiredPumpState;
        }

        usleep(1000000); // Sleep for 1 second
        ++iteration;
    }
}