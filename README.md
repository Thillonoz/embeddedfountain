# Embedded Fountain Project

This project details an intelligent, solar-powered fountain controlled by an ESP32 microcontroller. The fountain utilizes sensor data and Wi-Fi connectivity to manage pump operation, offering both automated and remote control capabilities. The system is designed with energy efficiency in mind, drawing power from a battery charged by a solar panel.

## Features

- Solar-Powered Operation: Runs on a battery charged by a solar panel, making it an eco-friendly and off-grid solution.
- Smart Pump Control: An ESP32-C6 microcontroller manages the fountain pump based on various parameters, including scheduled timings and sensor readings.
- Sensor Integration: Monitors crucial environmental factors:
  - Water Level: Ensures the pump operates safely and prevents dry running.
  - Wind Speed: Allows for dynamic adjustment of pump operation, potentially reducing splashing or conserving water during windy conditions.
- Time Synchronization: Connects to Wi-Fi to obtain accurate time for scheduled operations.
- MQTT Control: Enables remote control of the fountain pump via MQTT, offering flexibility in management.
- Modular Design: Housed in a custom-designed enclosure styled as a house, with provisions for future expansion like a wind turbine.

## Components

The following key components are used in this project:

- Microcontroller: ESP32-C6-DevKitC-1
- Relay Module: Relay 5V Optocoupler (for pump control)
- ESP32 Programmer: ESP32 Programmer UART-USB
- Power System: ECO-WORTHY 10W 12V Off-grid Solpanelsats with Lithium Battery and PWM Charge Controller
- Breadboard Power Supply: Power supply for breadboard 3.3/5V USB-C (for development/testing)
- Wind Sensor: Anemometer with Analog Output
- Water Level Sensor: Float switch

## Project Structure (Box/House)

The electronic components are housed within a custom-built enclosure designed to resemble a small house. The solar panel is strategically mounted on the roof of this "house" to maximize sunlight exposure. Future plans include the addition of a small wind turbine to further augment the power generation capabilities.

## Getting Started

Detailed instructions on setting up the hardware, flashing the ESP32 with the firmware, and configuring the Wi-Fi and MQTT settings will be provided here.

## Future Enhancements

- Integration of a wind turbine for additional power generation.
- Advanced data logging and visualization of sensor data.
- Implementation of a web interface for local control and monitoring.
- Predictive control based on weather forecasts to optimize pump operation and water conservation.

## Concept image (made by Sora)

![Sora concept image](/assets/images/concept.png)
