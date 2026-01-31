#include "PageManager.h"

#include <stddef.h>

static Page current_page = {0};

void PageManager_handle_input(ButtonCode button) {
    if (current_page.handle_input) {
        current_page.handle_input(button);
    }
}

void PageManager_tick(uint32_t delta_ms) {
    // Przekazujemy "puls" tylko jeśli strona ma zdefiniowaną funkcję on_tick
    if (current_page.on_tick) {
        current_page.on_tick(delta_ms);
    }
}

void PageManager_switch_page(Page* new_page) {
    // 1. Clean up old page
    if (current_page.exit) {
        current_page.exit();
    }

    // 2. Assign new page
    if (new_page) {
        current_page = *new_page;
    } else {
        // Reset to safe defaults
        current_page.handle_input = NULL;
        current_page.on_tick = NULL;
        current_page.exit = NULL;
    }
}