#include "wifi.h"
#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"

//  You have to make your own credentials.h file or remove the include line
#include "credentials.h"
// -----------------------------------------------------------------------

#ifndef SSID_KEY
#define SSID_KEY "ssid"
#endif
#ifndef PASS_KEY
#define PASS_KEY "pass"
#endif

#define NAMESPACE "wifi"

static volatile bool connected = false;

static void read_string(char *str, size_t len)
{
    int chr = 0;
    size_t i = 0;

    while (EOF != getchar())
    {
        ;
    }

    while (i < len)
    {
        chr = getchar();

        if (chr != EOF)
        {
            if (chr == '\n')
            {
                break;
            }
            else
            {
                putchar(chr);
                str[i] = chr;
                i++;
            }
        }
    }

    putchar('\n');
}

static bool console_get_credentials(char *ssid, char *pass)
{
    size_t len;
    bool status = false;
    nvs_handle_t handle = 0;

    ESP_ERROR_CHECK(esp_task_wdt_delete(xTaskGetIdleTaskHandle()));

    do
    {
        printf("Enter SSID: ");
        read_string(ssid, sizeof(((wifi_config_t *)NULL)->sta.ssid) - 1);
        len = strlen(ssid);
    } while (len == 0);

    do
    {
        printf("Enter Password: ");
        read_string(pass, sizeof(((wifi_config_t *)NULL)->sta.password) - 1);
        len = strlen(pass);
    } while (len == 0);

    if (ESP_OK == nvs_open(NAMESPACE, NVS_READWRITE, &handle))
    {
        if ((ESP_OK == nvs_set_str(handle, SSID_KEY, ssid)) && (ESP_OK == nvs_set_str(handle, PASS_KEY, pass)))
        {
            if (ESP_OK == nvs_commit(handle))
            {
                status = true;
            }
        }

        nvs_close(handle);
    }

    ESP_ERROR_CHECK(esp_task_wdt_add(xTaskGetIdleTaskHandle()));

    return status;
}

static bool nvs_get_credentials(char *ssid, char *pass)
{
    size_t len;
    bool status = false;
    nvs_handle_t handle = 0;

    if (ESP_OK != nvs_open(NAMESPACE, NVS_READONLY, &handle))
    {
        ESP_ERROR_CHECK(nvs_open(NAMESPACE, NVS_READWRITE, &handle));
        nvs_close(handle);
        ESP_ERROR_CHECK(nvs_open(NAMESPACE, NVS_READONLY, &handle));
    }

    if (ESP_OK == nvs_get_str(handle, SSID_KEY, NULL, &len)) // To get the string length
    {
        if (len <= sizeof(((wifi_config_t *)NULL)->sta.ssid))
        {
            if (ESP_OK == nvs_get_str(handle, SSID_KEY, ssid, &len))
            {
                if (ESP_OK == nvs_get_str(handle, PASS_KEY, NULL, &len)) // To get the string length
                {
                    if (len <= sizeof(((wifi_config_t *)NULL)->sta.password))
                    {
                        if (ESP_OK == nvs_get_str(handle, PASS_KEY, pass, &len))
                        {
                            status = true;
                        }
                    }
                }
            }
        }
    }
    nvs_close(handle);

    return status;
}

static void event_handler(void *, esp_event_base_t event_base, int32_t event_id, void *)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        connected = false;
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        connected = false;
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        connected = true;
    }
}

esp_err_t wifi_init()
{
    esp_err_t status = nvs_flash_init();
    if ((status == ESP_ERR_NVS_NO_FREE_PAGES) || (status == ESP_ERR_NVS_NEW_VERSION_FOUND))
    {
        status = nvs_flash_erase();

        if (status == ESP_OK)
        {
            status = nvs_flash_init();
        }
    }

    if (status != ESP_OK)
    {
        goto exit;
    }

    char ssid[sizeof(((wifi_config_t *)NULL)->sta.ssid)] = {0};
    char pass[sizeof(((wifi_config_t *)NULL)->sta.password)] = {0};

    if (strcmp(SSID_KEY, "ssid") == 0 && strcmp(PASS_KEY, "pass") == 0)
    {
        if (!nvs_get_credentials(ssid, pass))
        {
            memset(ssid, 0, sizeof(ssid));
            memset(pass, 0, sizeof(pass));

            if (!console_get_credentials(ssid, pass))
            {
                status = ESP_FAIL;
                goto exit;
            }
        }
    }
    else
    {
        // Use hardcoded credentials from credentials.h
        strncpy(ssid, SSID_KEY, sizeof(((wifi_config_t *)NULL)->sta.ssid) - 1);
        strncpy(pass, PASS_KEY, sizeof(((wifi_config_t *)NULL)->sta.password) - 1);
    }

    status = esp_netif_init();
    if (status != ESP_OK)
    {
        goto exit;
    }

    status = esp_event_loop_create_default();
    if (status != ESP_OK)
    {
        goto exit;
    }

    status = (NULL == esp_netif_create_default_wifi_sta()) ? ESP_FAIL : ESP_OK;
    if (status != ESP_OK)
    {
        goto exit;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    status = esp_wifi_init(&cfg);
    if (status != ESP_OK)
    {
        goto exit;
    }

    status = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL);
    if (status != ESP_OK)
    {
        goto exit;
    }

    status = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL);
    if (status != ESP_OK)
    {
        goto exit;
    }

    wifi_config_t wifi_config = {};
    memcpy(wifi_config.sta.ssid, ssid, sizeof(ssid));
    memcpy(wifi_config.sta.password, pass, sizeof(pass));

    status = esp_wifi_set_mode(WIFI_MODE_STA);
    if (status != ESP_OK)
    {
        goto exit;
    }

    status = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    if (status != ESP_OK)
    {
        goto exit;
    }

    status = esp_wifi_start();

exit:
    return status;
}

bool wifi_connected(void)
{
    return connected;
}

void wifi_reset(void)
{
    nvs_handle_t handle = 0;
    if (ESP_OK == nvs_open(NAMESPACE, NVS_READWRITE, &handle))
    {
        (void)nvs_erase_all(handle);
        (void)nvs_commit(handle);
        nvs_close(handle);
    }
    esp_restart();
}