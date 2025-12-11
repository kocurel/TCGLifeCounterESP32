// #ifndef CONTROLLER_H
// #define CONTROLLER_H
#include "Button.h"
#include "ViewModel.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"
#include "soc/soc_caps.h"

// The handler takes the command (argv[0]) plus arguments.
// Expected usage: 'button up' or 'button down'
int button_command_handler(int argc, char** argv) {
    // We expect the command name (argv[0]) + one argument (argv[1])
    if (argc != 2) {
        printf("Error: Missing argument. Usage: button [up|down|select]\n");
        return -1;
    }

    // Check the argument (argv[1]) using correct strcmp logic
    if (strcmp(argv[1], "up") == 0) {
        ViewModel_handle_input(BUTTON_CODE_UP);
    } else if (strcmp(argv[1], "down") == 0) {
        ViewModel_handle_input(BUTTON_CODE_DOWN);
    } else if (strcmp(argv[1], "left") == 0) {
        ViewModel_handle_input(BUTTON_CODE_LEFT);
    } else if (strcmp(argv[1], "right") == 0) {
        ViewModel_handle_input(BUTTON_CODE_RIGHT);
    } else if (strcmp(argv[1], "accept") == 0) {
        ViewModel_handle_input(BUTTON_CODE_ACCEPT);
    } else if (strcmp(argv[1], "cancel") == 0) {
        ViewModel_handle_input(BUTTON_CODE_CANCEL);
    } else if (strcmp(argv[1], "set") == 0) {
        ViewModel_handle_input(BUTTON_CODE_SET);
    } else if (strcmp(argv[1], "menu") == 0) {
        ViewModel_handle_input(BUTTON_CODE_MENU);
    } else {
        printf("Error: Invalid button argument '%s'.\n", argv[1]);
    }
    return 0;
}

esp_err_t ViewController_register_button_commands(void) {
    const esp_console_cmd_t command = {
        // Register the main command name
        .command = "btn",
        // Describe the full usage
        .help = "Simulate a button press. Usage: button [up|down|select]",
        .hint = "[up|down|select]",  // Optional hint for auto-completion
        // Point to the handler
        .func = &button_command_handler,
    };
    return esp_console_cmd_register(&command);
}

// #endif  // CONTROLLER_H
