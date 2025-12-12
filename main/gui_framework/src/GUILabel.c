#include <stdio.h>   // Required for snprintf
#include <string.h>  // Required for memset (optional but good practice)

#include "GUIComponent.h"
#include "GUIFramework.h"

// --- Private Delegate ---
// This function remains static so it's hidden from other files.
// It matches the signature: void (*draw)(GUIComponent* self)
static void GUILabel_draw(GUIComponent* self) {
    if (self == NULL) return;

    // 1. Cast generic component back to specific Label
    GUILabel* label = (GUILabel*)self;

    GUI_TRACE("GUILabel_draw", "Drawing GUILabel at address %p", self);

    // 2. Setup Render State
    GUIRenderer_set_font_size(label->font_size);

    // 3. Calculate Layout
    uint8_t text_width = GUIRenderer_get_string_width(label->text);
    int8_t ascent = GUIRenderer_get_ascent();
    int8_t descent = GUIRenderer_get_descent();
    int8_t text_height = ascent - descent;

    // 4. Draw Normal or Rotated
    if (!label->isUpsideDown) {
        GUIRenderer_rotate_text_disable();
        // Center text in the bounding box
        uint8_t x = self->x + (self->width - text_width) / 2;
        uint8_t y = self->y + text_height + (self->height - text_height) / 2;
        GUIRenderer_draw_str(x, y, label->text);
    } else {
        GUIRenderer_rotate_text_enable();
        // Calculate rotated origin
        uint8_t x = self->x + (self->width - text_width) / 2 + text_width;
        uint8_t y = self->y + (self->height - text_height) / 2;
        GUIRenderer_draw_str(x, y, label->text);
    }
}

// --- Public Initializer ---
// Replaces GUILabel_new().
// Assumes 'self' points to valid static/stack memory.
void GUILabel_init(GUILabel* self, const char* initial_text) {
    if (self == NULL) return;

    GUI_TRACE("GUILabel_init", "Initializing GUILabel at address %p", self);

    // 1. Initialize Base Component
    GUIComponent_init(&self->base);

    // 2. Assign V-Table (The Core Logic)
    self->base.draw = GUILabel_draw;
    self->base.layout = NULL;  // Labels usually don't layout children

    // 3. Initialize Label-Specific State
    self->font_size = 10;
    self->isUpsideDown = false;

    // 4. Set Text Safely (No strdup!)
    if (initial_text) {
        GUILabel_set_text(self, initial_text);
    } else {
        self->text[0] = '\0';  // Empty string
    }
}

// --- Public Methods ---

void GUILabel_set_text(GUILabel* self, const char* str) {
    if (self == NULL || str == NULL) return;

    // FIX: Replaced strdup (heap) with snprintf (static buffer)
    // Ensures we never overflow the LABEL_MAX_SIZE
    snprintf(self->text, LABEL_MAX_SIZE, "%s", str);
}

void GUILabel_set_font_size(GUILabel* self, uint8_t font_size) {
    if (self == NULL) return;
    self->font_size = font_size;
}

void GUILabel_upside_down_en(GUILabel* self, bool flag) {
    if (self == NULL) return;
    self->isUpsideDown = flag;
}