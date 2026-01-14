#include "Button.h"

#include <stddef.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "sdkconfig.h"

typedef struct {
    ButtonCode code;
    void (*callback)(void);
} Button;

static Button buttons[] = {
    {BUTTON_CODE_LEFT, NULL},   {BUTTON_CODE_RIGHT, NULL},
    {BUTTON_CODE_UP, NULL},     {BUTTON_CODE_DOWN, NULL},
    {BUTTON_CODE_ACCEPT, NULL}, {BUTTON_CODE_CANCEL, NULL},
    {BUTTON_CODE_SET, NULL},    {BUTTON_CODE_MENU, NULL}};

void Button_assign_callback(ButtonCode button, void (*callback)(void)) {
    buttons[button].callback = callback;
}

void Button_handle_press(ButtonCode button) {
    if (buttons[button].callback != NULL) {
        buttons[button].callback();
    }
}

bool Button_is_pressed(ButtonCode button) {
    int pin;
    switch (button) {
        case BUTTON_CODE_LEFT:
            pin = BUTTON_LEFT_PIN;
            break;
        case BUTTON_CODE_RIGHT:
            pin = BUTTON_RIGHT_PIN;
            break;
        case BUTTON_CODE_UP:
            pin = BUTTON_UP_PIN;
            break;
        case BUTTON_CODE_DOWN:
            pin = BUTTON_DOWN_PIN;
            break;
        case BUTTON_CODE_ACCEPT:
            pin = BUTTON_ACCEPT_PIN;
            break;
        case BUTTON_CODE_CANCEL:
            pin = BUTTON_CANCEL_PIN;
            break;
        case BUTTON_CODE_SET:
            pin = BUTTON_SET_PIN;
            break;
        case BUTTON_CODE_MENU:
            pin = BUTTON_MENU_PIN;
            break;
        default:
            return false;  // Invalid button code
    }
    return gpio_get_level(pin) == 0;  // Assuming active low buttons
}

static void IRAM_ATTR button_isr_handler(void* arg) {
    ButtonCode button = (ButtonCode)(uintptr_t)arg;
    Button_handle_press(button);
}

void Button_init(void) {
    // Initialize GPIO pins for buttons
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,  // Interrupt on falling edge
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_LEFT_PIN) | (1ULL << BUTTON_RIGHT_PIN) |
                        (1ULL << BUTTON_UP_PIN) | (1ULL << BUTTON_DOWN_PIN) |
                        (1ULL << BUTTON_ACCEPT_PIN) |
                        (1ULL << BUTTON_CANCEL_PIN) | (1ULL << BUTTON_SET_PIN) |
                        (1ULL << BUTTON_MENU_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,  // Enable pull-up resistors
    };
    gpio_config(&io_conf);

    // Install GPIO ISR service
    gpio_install_isr_service(0);

    // Attach ISR handlers for each button
    gpio_isr_handler_add(BUTTON_LEFT_PIN, button_isr_handler,
                         (void*)(uintptr_t)BUTTON_CODE_LEFT);
    gpio_isr_handler_add(BUTTON_RIGHT_PIN, button_isr_handler,
                         (void*)(uintptr_t)BUTTON_CODE_RIGHT);
    gpio_isr_handler_add(BUTTON_UP_PIN, button_isr_handler,
                         (void*)(uintptr_t)BUTTON_CODE_UP);
    gpio_isr_handler_add(BUTTON_DOWN_PIN, button_isr_handler,
                         (void*)(uintptr_t)BUTTON_CODE_DOWN);
    gpio_isr_handler_add(BUTTON_ACCEPT_PIN, button_isr_handler,
                         (void*)(uintptr_t)BUTTON_CODE_ACCEPT);
    gpio_isr_handler_add(BUTTON_CANCEL_PIN, button_isr_handler,
                         (void*)(uintptr_t)BUTTON_CODE_CANCEL);
    gpio_isr_handler_add(BUTTON_SET_PIN, button_isr_handler,
                         (void*)(uintptr_t)BUTTON_CODE_SET);
    gpio_isr_handler_add(BUTTON_MENU_PIN, button_isr_handler,
                         (void*)(uintptr_t)BUTTON_CODE_MENU);
}