#include <stdio.h>
#include <string.h>

#include "ViewController.h"
#include "argtable3/argtable3.h"
#include "console_init.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_dev.h"
#include "gui_framework/include/test/GUItests.h"
#include "linenoise/linenoise.h"
#include "soc/soc_caps.h"

int run_tests() {
    UNITY_BEGIN();
    RUN_TEST(test_VBox_is_properly_initiated);
    RUN_TEST(test_HBox_is_properly_initiated);
    RUN_TEST(test_Label_is_properly_initiated);
    RUN_TEST(test_Container_add_child_recursive_delete);
    RUN_TEST(test_Nested_Container_Layout);
    RUN_TEST(test_Safety_Null_Inputs);
    RUN_TEST(test_VBox_Full_Height_Distribution);
    RUN_TEST(test_HBox_Full_Width_Distribution);
    return UNITY_END();
}

void app_main(void) {
    // run_tests();
    ViewModel_init();
    //   comamnd line interface
    initialize_console();

    /* Register commands */
    esp_console_register_help_command();
    // register_system_common();
    ViewController_register_button_commands();
    // register_system_sleep();

    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    const char* prompt = LOG_COLOR_I "> " LOG_RESET_COLOR;

    printf(
        "\n"
        "This is an example of ESP-IDF console component.\n"
        "Type 'help' to get the list of commands.\n"
        "Use UP/DOWN arrows to navigate through command history.\n"
        "Press TAB when typing command name to auto-complete.\n"
        "Press Enter or Ctrl+C will terminate the console environment.\n");

    /* Main loop */
    while (true) {
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char* line = linenoise(prompt);
        if (line == NULL) { /* Break on EOF or error */
            break;
        }
        /* Add the command to the history if not empty*/
        if (strlen(line) > 0) {
            linenoiseHistoryAdd(line);
        }

        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret,
                   esp_err_to_name(ret));
        } else if (err != ESP_OK) {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }

    ESP_LOGE(TAG, "Error or end-of-input, terminating console");
    esp_console_deinit();
}
