#ifndef WATER_LEVEL_H
#define WATER_LEVEL_H

#include <stdbool.h>

#define WATER_LEVEL_HIGH 0          // Water level is high (closed)
#define WATER_LEVEL_LOW 1           // Water level is low (open)
#define WATER_LEVEL_UNINITIALIZED 2 // Water level sensor not initialized
#define LOW 0
#define HIGH 1

/**
 * @brief Initialize the water level sensor module.
 *
 * @param pin is the pin number used to connect the water level sensor.
 * @return true
 * @return false
 */
bool water_level_init(int pin);

/**
 * @brief returns the state of the water level sensor, normally open.
 *
 * @return int 0 = water level is high (closed), 1 = water level is low (open), 2 = uninitialized.
 */
int water_level_get_state();

#endif // WATER_LEVEL_H