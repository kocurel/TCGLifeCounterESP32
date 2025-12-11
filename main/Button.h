#ifndef BUTTON_H
#define BUTTON_H

#define BUTTTON_LEFT 0x00
#define BUTTON_RIGHT 0x01
#define BUTTON_UP 0x02
#define BUTTON_DOWN 0x03
#define BUTTON_ACCEPT 0x04
#define BUTTON_CANCEL 0x05
#define BUTTON_SET 0x06
#define BUTTON_MENU 0x07

#define BUTTON_LEFT_PIN 4
#define BUTTON_RIGHT_PIN 5
#define BUTTON_UP_PIN 18
#define BUTTON_DOWN_PIN 19
#define BUTTON_ACCEPT_PIN 21
#define BUTTON_CANCEL_PIN 22
#define BUTTON_SET_PIN 23
#define BUTTON_MENU_PIN 25

typedef enum {
    BUTTON_CODE_LEFT = BUTTTON_LEFT,
    BUTTON_CODE_RIGHT = BUTTON_RIGHT,
    BUTTON_CODE_UP = BUTTON_UP,
    BUTTON_CODE_DOWN = BUTTON_DOWN,
    BUTTON_CODE_ACCEPT = BUTTON_ACCEPT,
    BUTTON_CODE_CANCEL = BUTTON_CANCEL,
    BUTTON_CODE_SET = BUTTON_SET,
    BUTTON_CODE_MENU = BUTTON_MENU
} ButtonCode;

typedef struct Button;

void Button_assign_callback(ButtonCode button, void (*callback)(void));

void Button_handle_press(ButtonCode button);

#endif  // BUTTON_H