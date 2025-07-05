#ifndef BSP_H
#define BSP_H

#include <stdint.h>
#include <stdbool.h>

#define BSP_LOW 0
#define BSP_HIGH 1
#define BSP_PULL_UP_DISABLE 0
#define BSP_PULL_UP_ENABLE 1
#define BSP_MODE_INPUT 1
#define BSP_MODE_OUTPUT 2

bool bsp_pin_config(int pin, int mode, int pull);

int bsp_pin_read(int pin);

int bsp_pin_write(int pin, int value);

#endif /* BSP_H */
