#include "battery.h"

#include "Debug.h"
#include "System.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

/* --- Configuration --- */
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_3
#define BATTERY_ADC_ATTEN ADC_ATTEN_DB_12
#define NO_OF_SAMPLES 64  // Oversampling count to reduce noise

typedef struct {
    uint16_t voltage;
    uint8_t percentage;
} battery_level_t;

/* Standard discharge curve for 3.7V LiPo */
static const battery_level_t lipo_table[] = {
    {4200, 100}, {4100, 100}, {4000, 90},
    {3800, 60},  {3700, 40},  {3600, 15},  // Discharge curve drops faster here
    {3420, 5},                             // Critical low warning area
    {3300, 0}                              // Hardware floor
};

/* --- Internal Logic --- */

/**
 * Converts measured pin voltage to battery percentage using linear
 * interpolation.
 * @param pin_mv Measured voltage at the GPIO pin after the voltage divider.
 */
int get_battery_percentage(int pin_mv) {
    // 1. Convert Pin Voltage to actual Battery Voltage
    // Ratio based on resistor divider: 4200mV (Max Bat) / 1370mV (Max Pin) =
    // ~3.066
    int battery_mv = (pin_mv * 4200) / 1370;

    // 2. Handle Clamping
    if (battery_mv >= 4200) return 100;
    if (battery_mv <= 3000) return 0;

    // 3. Lookup Table Interpolation
    int table_size = sizeof(lipo_table) / sizeof(lipo_table[0]);
    for (int i = 0; i < table_size - 1; i++) {
        if (battery_mv >= lipo_table[i + 1].voltage) {
            int v_high = lipo_table[i].voltage;
            int v_low = lipo_table[i + 1].voltage;
            int p_high = lipo_table[i].percentage;
            int p_low = lipo_table[i + 1].percentage;

            // Linear interpolation calculation
            return p_low +
                   ((battery_mv - v_low) * (p_high - p_low) / (v_high - v_low));
        }
    }

    return 0;
}

/**
 * Performs a hardware ADC read on GPIO3 with multisampling.
 * Returns voltage measured at the pin in millivolts (mV).
 */
int read_battery_voltage_mv(void) {
    // 1. Initialize ADC Unit
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    // 2. Configure ADC Channel
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = BATTERY_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(adc1_handle, BATTERY_ADC_CHANNEL, &config));

    // 3. Multisampling Read Loop
    int adc_raw = 0;
    int accumulator = 0;

    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        ESP_ERROR_CHECK(
            adc_oneshot_read(adc1_handle, BATTERY_ADC_CHANNEL, &adc_raw));
        accumulator += adc_raw;
        esp_rom_delay_us(500);
    }

    int average_raw = accumulator / NO_OF_SAMPLES;

    // 4. Cleanup ADC Resources
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));

    // 5. Convert Raw ADC value to Voltage (Approximation)
    // Map 12-bit range (4095) to full scale voltage (approx 2950mV at 12dB
    // atten)
    int pin_mv = (average_raw * 2950) / 4095;
    int bat_mv = (pin_mv * 4200) / 1370;
    int pct = get_battery_percentage(pin_mv);

    ESP_LOGI("BATTERY", "Pin: %d mV | Bat: %d mV | Level: %d%%", pin_mv, bat_mv,
             pct);

    return pin_mv;
}

/* --- Public API & Tasks --- */

/**
 * High-level function to read voltage and update system state.
 */
void read_battery_level() {
    int mv = read_battery_voltage_mv();
    int battery_level = get_battery_percentage(mv);
    System_set_battery_percentage(battery_level);
}

/**
 * Periodic task to monitor battery levels.
 */
void battery_scan_task(void* pvParameters) {
    while (1) {
        read_battery_level();
        // Check battery every 30 seconds
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}