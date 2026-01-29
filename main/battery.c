#include "battery.h"

#include "Debug.h"
#include "System.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

// Constants
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_3
#define BATTERY_ADC_ATTEN ADC_ATTEN_DB_12
#define NO_OF_SAMPLES 64  // Oversampling

typedef struct {
    uint16_t voltage;
    uint8_t percentage;
} battery_level_t;

// Standard discharge curve for 3.7V LiPo (approximate)
static const battery_level_t lipo_table[] = {
    {4200, 100},
    {4100, 100},
    {4000, 90},
    {3800, 60},
    {3700, 40},
    {3600, 15},  // Discharge curve starts dropping
                 // faster here
    {3420, 5},   // Critical low warning area
    {3300, 0}    // Hard Floor (Regulator stops regulating)
};

/**
 * @brief Converts pin voltage (mV) to battery percentage (0-100%)
 * @param pin_mv Voltage measured at the GPIO pin (e.g., 1344)
 */
int get_battery_percentage(int pin_mv) {
    // 1. Convert Pin Voltage -> Actual Battery Voltage
    // Ratio based on your divider: 4200mV (Max Bat) / 1370mV (Max Pin) = ~3.066
    int battery_mv = (pin_mv * 4200) / 1370;

    // 2. Handle Edge Cases
    if (battery_mv >= 4200) return 100;
    if (battery_mv <= 3000) return 0;

    // 3. Interpolate from Lookup Table
    for (int i = 0; i < sizeof(lipo_table) / sizeof(lipo_table[0]) - 1; i++) {
        if (battery_mv >= lipo_table[i + 1].voltage) {
            // We found the range [i, i+1]
            // Linear interpolation for smooth transition
            int v_high = lipo_table[i].voltage;
            int v_low = lipo_table[i + 1].voltage;
            int p_high = lipo_table[i].percentage;
            int p_low = lipo_table[i + 1].percentage;

            return p_low +
                   ((battery_mv - v_low) * (p_high - p_low) / (v_high - v_low));
        }
    }

    return 0;
}

/**
 * @brief Reads the battery voltage from GPIO3.
 * * Uses multisampling to reduce noise.
 * Returns voltage in millivolts (mV).
 */
int read_battery_voltage_mv(void) {
    // 1. Resource Handle
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };

    // 2. Initialize the ADC Unit
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    // 3. Configure the Channel (GPIO3)
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = BATTERY_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(adc1_handle, BATTERY_ADC_CHANNEL, &config));

    // 4. Calibration Setup (Optional but recommended for accuracy)
    // In a real product, you would initialize the calibration scheme here using
    // esp_adc_cal. For this example, we will read raw values.

    // 5. Read and Average (Multisampling)
    int adc_raw = 0;
    int accumulator = 0;

    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        ESP_ERROR_CHECK(
            adc_oneshot_read(adc1_handle, BATTERY_ADC_CHANNEL, &adc_raw));
        accumulator += adc_raw;
        // Simple delay to space out readings (read "slowly")
        esp_rom_delay_us(500);
    }

    int average_raw = accumulator / NO_OF_SAMPLES;

    // 6. Cleanup (Free the handle)
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));

    // 7. Convert Raw to Voltage (Approximate linear calculation)
    int pin_mv = (average_raw * 2950) / 4095;

    int pct = get_battery_percentage(pin_mv);

    int bat_mv = (pin_mv * 4200) / 1370;
    ESP_LOGI("BATTERY", "Pin: %d mV | Bat: %d mV | Level: %d%%", pin_mv, bat_mv,
             pct);
    return pin_mv;
}

void read_battery_level() {
    LOG_DEBUG("read_battery_level", "Reading battery level");
    int mv = read_battery_voltage_mv();
    int battery_level = get_battery_percentage(mv);
    System_set_battery_percentage(battery_level);
}

void battery_scan_task(void* pvParameters) {
    while (1) {
        read_battery_level();
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}