#include "pins.h"
#include "battery_voltage.h"
#include <esp_log.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali_scheme.h>

#define ADC_ATTENUATION ADC_ATTEN_DB_12
#define ADC_RESOLUTION ADC_BITWIDTH_DEFAULT

const static char *TAG = "Battery Voltage";

static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t cali_handle = NULL;

static int millivolts;
static const float R1 = 99000.0f; // Resistor R1 value in ohms
static const float R2 = 24600.0f; // Resistor R2 value in ohms
static float v_adc;               // Measured voltage at the ADC pin

void battery_voltage_init(void)
{
    ESP_LOGI(TAG, "ADC1 Initialization");
    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_config, &adc_handle));

    ESP_LOGI(TAG, "ADC1 Configuration");
    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_RESOLUTION,
        .atten = ADC_ATTENUATION,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, BATTERY_VOLTAGE_CHANNEL, &channel_config));

    ESP_LOGI(TAG, "ADC1 Calibration");
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = BATTERY_VOLTAGE_CHANNEL,
        .atten = ADC_ATTENUATION,
        .bitwidth = ADC_RESOLUTION,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));
}

void battery_voltage_run(void)
{
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, BATTERY_VOLTAGE_CHANNEL, &millivolts));
    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, BATTERY_VOLTAGE_CHANNEL, millivolts);

    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, millivolts, &v_adc));
    ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC_CHANNEL_0, v_adc);

    v_bat = v_adc * (R1 + R2) / R2;
}