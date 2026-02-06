#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include <stdint.h>

#include "GUIFramework.h"
#include "model/Button.h"

// The generic interface for any screen in your app
typedef struct Page {
    // Wywoływane, gdy naciśnięto przycisk
    void (*handle_input)(ButtonCode);

    // [NOWOŚĆ] Wywoływane cyklicznie w pętli głównej
    // delta_ms: czas w ms, który upłynął od ostatniego wywołania (do
    // animacji/timerów)
    void (*on_tick)(uint32_t delta_ms);
} Page;

// Call this from your main loop (Producer)
void PageManager_handle_input(ButtonCode button);

// Call this from your main loop periodically (e.g. every 10-50ms)
void PageManager_tick(uint32_t delta_ms);

// Call this from inside a page to switch to a new one
void PageManager_switch_page(Page* new_page);

#endif