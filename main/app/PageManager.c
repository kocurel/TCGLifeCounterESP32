#include "PageManager.h"

#include <stddef.h>

/* --- Private State --- */

/** * Stores the currently active page's function pointers.
 * Initialized to safe defaults (NULL).
 */
static Page current_page = {0};

/* --- Input and Logic Propagation --- */

/**
 * Forwards hardware button events to the active page's handler.
 */
void PageManager_handle_input(ButtonCode button) {
    if (current_page.handle_input) {
        current_page.handle_input(button);
    }
}

/**
 * Propagates the system clock tick to the active page.
 * Used for timed events, animations, or auto-commit timers.
 */
void PageManager_tick(uint32_t delta_ms) {
    if (current_page.on_tick) {
        current_page.on_tick(delta_ms);
    }
}

/* --- State Transitions --- */

/**
 * Performs a safe context switch between two pages.
 * Ensures the previous page's cleanup logic is executed.
 */
void PageManager_switch_page(Page* new_page) {
    // 1. Execute cleanup logic for the outgoing page
    if (current_page.exit) {
        current_page.exit();
    }

    // 2. Assign the new page's configuration
    if (new_page) {
        current_page = *new_page;
    } else {
        // Reset to safe defaults to prevent null pointer execution
        current_page.handle_input = NULL;
        current_page.on_tick = NULL;
        current_page.exit = NULL;
    }
}