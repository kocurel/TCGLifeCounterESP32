#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include "GUIFramework.h"
#include "model/Button.h"

// The generic interface for any screen in your app
typedef struct Page {
    void (*handle_input)(ButtonCode);
    void (*exit)(void);
} Page;

// Call this from your main loop
void PageManager_handle_input(ButtonCode button);

// Call this from inside a page to switch to a new one
void PageManager_switch_page(Page* new_page);

#endif