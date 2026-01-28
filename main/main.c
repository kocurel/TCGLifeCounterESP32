#include <stdio.h>
#include <string.h>

#include "ViewController.h"
#include "app/pages/MainPage.h"
#include "argtable3/argtable3.h"
#include "console_init.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_dev.h"
// #include "gui_framework/include/test/GUItests.h"
#include "battery.h"
#include "keypad.h"
#include "linenoise/linenoise.h"
#include "model/Game.h"
#include "soc/soc_caps.h"

// int run_tests() {
//     // UNITY_BEGIN();
//     // RUN_TEST(test_VBox_is_properly_initiated);
//     // RUN_TEST(test_HBox_is_properly_initiated);
//     // RUN_TEST(test_Label_is_properly_initiated);
//     // RUN_TEST(test_Container_add_child_recursive_delete);
//     // RUN_TEST(test_Nested_Container_Layout);
//     // RUN_TEST(test_Safety_Null_Inputs);
//     // RUN_TEST(test_VBox_Full_Height_Distribution);
//     // RUN_TEST(test_HBox_Full_Width_Distribution);
//     // return UNITY_END();
// }

#define DISP_EN_PIN 21


// I2C Configuration (XIAO ESP32-C3)
// 0x3C shifted left by 1 is 0x78
#define I2C_DISPLAY_ADDR_SHIFTED 0x78
#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 7


// #include "driver/i2c_master.h"
#include "esp_log.h"

#define I2C_MASTER_NUM              0      
#define OLED_ADDR                   0x3C

void app_main(void) {
    // run_tests();

    // Comamnd line interface
    ESP_LOGI("MAIN", "System initialized. Console running in background.");
    xTaskCreate(console_task, "console_cli", 5120, NULL, 5, NULL);

    // Display
    GUIRenderer_init();

    // Keypad
    keypad_init();
    xTaskCreate(keypad_scan_task, "keypad_task", 2048, NULL, 5, NULL);

    // Start system
    Game_init();
    MainPage_enter();

    char received_key;
    while (1) {
        if (xQueueReceive(get_keypad_queue(), &received_key, portMAX_DELAY)) {
            printf("Key pressed: %c\n", received_key);
            int bat = read_battery_level();
            printf("Battery level: %d\n", bat);
            
            // read_battery_voltage_mv();
        }
    }
}