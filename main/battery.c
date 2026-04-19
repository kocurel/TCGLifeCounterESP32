#include "battery.h"

#include "Debug.h"
#include "System.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define BATTERY_ADC_CHANNEL ADC_CHANNEL_3
#define BATTERY_ADC_ATTEN ADC_ATTEN_DB_12
#define NO_OF_SAMPLES 64

typedef struct {
    uint16_t voltage;
    uint8_t percentage;
} battery_level_t;

/* Discharge curve mapping for a standard 3.7V LiPo battery */
static const battery_level_t lipo_table[] = {
    {4200, 100}, {4100, 100}, {4000, 90}, {3800, 60},
    {3700, 40},  {3600, 15},  {3420, 5},  {3300, 0}};

/**
 * Interpolates battery percentage based on calculated battery voltage
 */
int get_battery_percentage(int pin_mv) {
    /* Convert measured pin voltage to actual battery voltage via resistor
     * divider ratio */
    const int battery_mv = (pin_mv * 4200) / 1370;

    if (battery_mv >= 4200) return 100;
    if (battery_mv <= 3000) return 0;

    const int table_size = sizeof(lipo_table) / sizeof(lipo_table[0]);
    for (int i = 0; i < table_size - 1; i++) {
        if (battery_mv >= lipo_table[i + 1].voltage) {
            const int v_high = lipo_table[i].voltage;
            const int v_low = lipo_table[i + 1].voltage;
            const int p_high = lipo_table[i].percentage;
            const int p_low = lipo_table[i + 1].percentage;

            /* Linear interpolation between discharge curve points */
            return p_low +
                   ((battery_mv - v_low) * (p_high - p_low) / (v_high - v_low));
        }
    }

    return 0;
}

/**
 * Executes a hardware ADC read with oversampling for noise reduction
 */
int read_battery_voltage_mv(void) {
    adc_oneshot_unit_handle_t adc1_handle;
    const adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    const adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = BATTERY_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(adc1_handle, BATTERY_ADC_CHANNEL, &config));

    int adc_raw = 0;
    int accumulator = 0;

    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        ESP_ERROR_CHECK(
            adc_oneshot_read(adc1_handle, BATTERY_ADC_CHANNEL, &adc_raw));
        accumulator += adc_raw;
        esp_rom_delay_us(500);
    }

    const int average_raw = accumulator / NO_OF_SAMPLES;
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));

    /* Map 12-bit ADC range to voltage; 2950mV is the approx full-scale at 12dB
     * attenuation */
    const int pin_mv = (average_raw * 2950) / 4095;
    const int bat_mv = (pin_mv * 4200) / 1370;
    const int pct = get_battery_percentage(pin_mv);

    ESP_LOGI("BATTERY", "Pin: %d mV | Bat: %d mV | Level: %d%%", pin_mv, bat_mv,
             pct);

    return pin_mv;
}

void read_battery_level() {
    const int mv = read_battery_voltage_mv();
    const int battery_level = get_battery_percentage(mv);
    System_set_battery_percentage(battery_level);
}

/**
 * Background task for periodic battery monitoring
 */
void battery_scan_task(void* pvParameters) {
    while (1) {
        read_battery_level();
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}