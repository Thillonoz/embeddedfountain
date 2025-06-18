#ifndef WIFI_H
#define WIFI_H

#include <esp_err.h>
#include <stdbool.h>

/**
 * @brief This function is used to read the wifi credentials from the nvs flash memory and use
 *        the credentials to connect the wifi module to a wifi network. You can hardcode the credentials
 *        in a credentials.h file in the wifi folder. If there is no wifi credential stored in the flash memory
 *        or there is no credentials.h file, it asks the user to enter ssid and password in the terminal
 *        and then stores them in the flash memory.
 *
 * @return esp_err_t ESP_OK if there is no error; otherwise ESP_ERR_*
 */
esp_err_t wifi_init();

/**
 * @brief This function is used to check if the wifi module is connected to the wifi network or not.
 *
 * @return true if the wifi module is connected to a wifi network; otherwise false
 */
bool wifi_connected(void);

/**
 * @brief This function is used to delete the stored wifi cridentials in the
 *        flash memory and then reset the microcontroller.
 *
 */
void wifi_reset(void);

#endif