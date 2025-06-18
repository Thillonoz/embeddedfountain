#include "water-level.h"
#include "bsp.h"

static int _pin = 0;
static bool initialized = false;

bool water_level_init(int pin)
{
    _pin = pin;
    initialized = bsp_pin_config(pin, BSP_MODE_INPUT, BSP_PULL_UP);
    return initialized;
}

int water_level_get_state()
{
    int state = WATER_LEVEL_UNINITIALIZED;
    if (initialized)
    {
        state = bsp_pin_read(_pin);
        if (state == LOW)
            state = WATER_LEVEL_HIGH; // Water level is high (closed)
        else if (state == HIGH)
            state = WATER_LEVEL_LOW; // Water level is low (open)
    }
    return state;
}