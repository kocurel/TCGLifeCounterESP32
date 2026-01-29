// #include "ViewController.h"

// #include <memory.h>
// #include <stdlib.h>

// #include "Game.h"
// typedef enum { PAGE_MAIN, PAGE_SELECT_OPERATION, PAGE_SET_VALUE } Page;

// typedef enum { MODE_NONE, MODE_ADD, MODE_SUB, MODE_SET } Mode;

// #define BUTTON_LEFT 0x00
// #define BUTTON_RIGHT 0x01
// #define BUTTON_UP 0x02
// #define BUTTON_DOWN 0x03

// #define BUTTON_ACCEPT 0x10
// #define BUTTON_CANCEL 0x11
// #define BUTTON_SET 0x12
// #define BUTTON_MENU 0x13

// typedef struct {
//     Page current_page;
//     Mode curent_mode;
//     int player_id;
// } State;

// struct ViewController {
//     View* view;
//     Game* game;
//     State state;
// };

// void ViewController_init(ViewController* controller) {
//     controller->game = (Game*)malloc(sizeof(Game));
//     GameSettings settings = GameSettings_create(4, 20, NULL, 0);
//     Game_init(controller->game, &settings);
//     controller->view = (Game*)malloc(sizeof(View));
//     View_initialize(controller->view);
// }

// void State_initialize(State* state) {
//     state->curent_mode = MODE_NONE;
//     state->current_page = PAGE_MAIN;
//     int player_id = 0;
// }

// void Handle_button_pressed_event(int button_pressed) {
//     switch (button_pressed) {
//         case BUTTON_UP:
//             /* code */
//             break;
//         case BUTTON_DOWN:
//             break;
//         case BUTTON_LEFT:
//             break;
//         case BUTTON_RIGHT:
//             break;
//         default:
//             break;
//     }
// }
