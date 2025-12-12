#ifndef GUILIST_H
#define GUILIST_H

#include "GUIFramework.h"

typedef struct GUIList GUIList;

// Delegate: Added void* context so the function knows where to look
typedef struct {
    void* context;
    const char* (*get_item)(void* context, uint32_t index);
    uint32_t (*get_count)(void* context);
} GUIListDelegate;

// Public API
GUIList* GUIList_new(GUIListDelegate delegate);
void GUIList_recalculate_layout(
    GUIList* self);  // Call this after setting size/font

void GUIList_scroll_next_page(GUIList* self);

#endif  // GUILIST_H