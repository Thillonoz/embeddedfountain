#ifndef BATTERY_VOLTAGE_H
#define BATTERY_VOLTAGE_H

float v_bat; // Actual battery voltage

/**
 * @brief Initialize the battery voltage monitoring system.
 * This function sets up the ADC and calibration for reading battery voltage.
 */
void battery_voltage_init(void);

/**
 * @brief Run the battery voltage monitoring system.
 * This function continuously reads the battery voltage and prints the value to the console.
 */
void battery_voltage_run(void);

#endif // BATTERY_VOLTAGE_H