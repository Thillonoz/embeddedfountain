#include "bsp.h"
#include "button.h"
#include <stddef.h>

#define SAMPLES (5U)

static int _pin = 0;
static bool initialized = false;
static int sample_buffer[SAMPLES];
static int sample_index = 0;
static int stable_state = BUTTON_RELEASED;
static int last_reported_state = BUTTON_RELEASED;
static int edge_event = BUTTON_UNINITITIALIZED;

bool button_init(int pin)
{
    _pin = pin;
    initialized = bsp_pin_config(pin, BSP_MODE_INPUT, BSP_PULL_UP);
    for (size_t i = 0; i < SAMPLES; ++i)
        sample_buffer[i] = BUTTON_RELEASED;
    sample_index = 0;
    stable_state = BUTTON_RELEASED;
    last_reported_state = BUTTON_RELEASED;
    edge_event = BUTTON_UNINITITIALIZED;
    return initialized;
}

void button_update_state(void)
{
    if (initialized)
    {
        int val = bsp_pin_read(_pin);
        sample_buffer[sample_index] = val;
        sample_index = (sample_index + 1) % SAMPLES;

        int first = sample_buffer[0];
        bool stable = true;
        for (size_t i = 1; i < SAMPLES; ++i)
        {
            if (sample_buffer[i] != first)
            {
                stable = false;
                break;
            }
        }
        if (stable && first != stable_state)
        {
            if (first == BUTTON_PRESSED && stable_state == BUTTON_RELEASED)
                edge_event = BUTTON_FALLING_EDGE;
            else if (first == BUTTON_RELEASED && stable_state == BUTTON_PRESSED)
                edge_event = BUTTON_RISING_EDGE;
            stable_state = first;
        }
    }
}

int button_get_state(void)
{
    int result = BUTTON_UNINITITIALIZED;
    if (initialized)
    {
        if (edge_event != BUTTON_UNINITITIALIZED)
        {
            result = edge_event;
            edge_event = BUTTON_UNINITITIALIZED;
        }
        else
        {
            result = stable_state;
        }
    }
    return result;
}