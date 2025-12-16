#include "argtable3/argtable3.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_dev.h"
#include "freertos/FreeRTOS.h"
#include "linenoise/linenoise.h"
#include "sdkconfig.h"
#include "soc/soc_caps.h"

void initialize_console() {
    /* Drain stdout before reconfiguring it */
    fflush(stdout);
    fsync(fileno(stdout));

    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM,
                                              ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM,
                                              ESP_LINE_ENDINGS_CRLF);

    /* Configure UART. Note that REF_TICK is used so that the baud rate
    remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config = {
        .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
#if SOC_UART_SUPPORT_REF_TICK
        .source_clk = UART_SCLK_REF_TICK,
#elif SOC_UART_SUPPORT_XTAL_CLK
        .source_clk = UART_SCLK_XTAL,
#endif
    };
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK(
        uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(
        uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config));

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    /* Initialize the console */
    esp_console_config_t console_config = {.max_cmdline_args = 8,
                                           .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
                                           .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*)&esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);

    /* Set command maximum length */
    linenoiseSetMaxLineLen(console_config.max_cmdline_length);

    /* Don't return empty lines */
    linenoiseAllowEmpty(false);
}

// #include "driver/usb_serial_jtag.h"
// #include "esp_vfs_usb_serial_jtag.h"
// // Keep your other includes (argtable3, esp_console, etc.)

// void initialize_console() {
//     /* Drain stdout before reconfiguring it */
//     fflush(stdout);
//     fsync(fileno(stdout));

//     /* Disable buffering on stdin */
//     setvbuf(stdin, NULL, _IONBF, 0);

//     /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
//     esp_vfs_dev_usb_serial_jtag_set_rx_line_endings(ESP_LINE_ENDINGS_CR);

//     /* Move the caret to the beginning of the next line on '\n' */
//     esp_vfs_dev_usb_serial_jtag_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

//     /* Install USB-SERIAL-JTAG driver for interrupt-driven reads and writes
//     */ usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
//         .tx_buffer_size = 256,
//         .rx_buffer_size = 256,
//     };
//     ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_config));

//     /* Tell VFS to use USB-SERIAL-JTAG driver */
//     esp_vfs_usb_serial_jtag_use_driver();

//     /* Initialize the console */
//     esp_console_config_t console_config = {.max_cmdline_args = 8,
//                                            .max_cmdline_length = 256,
// #if CONFIG_LOG_COLORS
//                                            .hint_color = atoi(LOG_COLOR_CYAN)
// #endif
//     };
//     ESP_ERROR_CHECK(esp_console_init(&console_config));

//     /* Configure linenoise line completion library */
//     linenoiseSetMultiLine(1);
//     linenoiseSetCompletionCallback(&esp_console_get_completion);
//     linenoiseSetHintsCallback((linenoiseHintsCallback*)&esp_console_get_hint);
//     linenoiseHistorySetMaxLen(100);
//     linenoiseSetMaxLineLen(console_config.max_cmdline_length);
//     linenoiseAllowEmpty(false);
// }