#include "GUIList.h"

#include <stdlib.h>  // For malloc

struct GUIList {
    GUIComponent base;
    GUIListDelegate delegate;

    // Internal State (The source of truth)
    int selected_index;  // 0 to Total-1
    int scroll_offset;   // 0 to Total-1 (Top visible item)
    int visible_rows;    // Calculated from height / font_height
    int row_height;
    uint8_t font_size;
};

// Helper: Ensure the selected item is actually visible
static void GUIList_ensure_visible(GUIList* self) {
    // Case 1: Selection is above the viewport
    if (self->selected_index < self->scroll_offset) {
        self->scroll_offset = self->selected_index;
    }

    // Case 2: Selection is below the viewport
    // The last visible index is (scroll_offset + visible_rows - 1)
    int bottom_visible = self->scroll_offset + self->visible_rows - 1;

    if (self->selected_index > bottom_visible) {
        // Scroll down just enough to put selection at the bottom
        self->scroll_offset = self->selected_index - (self->visible_rows - 1);
    }
}

void GUIList_move_down(GUIList* self) {
    int total_items = self->delegate.get_count(self->delegate.context);

    // 1. Logic: Move Selection
    if (self->selected_index < total_items - 1) {
        self->selected_index++;
    } else {
        self->selected_index = 0;  // Wrap to top
        self->scroll_offset = 0;   // Reset scroll
    }

    // 2. Logic: Auto-calculate Page/Offset
    GUIList_ensure_visible(self);
}

void GUIList_move_up(GUIList* self) {
    int total_items = self->delegate.get_count(self->delegate.context);

    // 1. Logic: Move Selection
    if (self->selected_index > 0) {
        self->selected_index--;
    } else {
        self->selected_index = total_items - 1;  // Wrap to bottom
    }

    // 2. Logic: Auto-calculate Page/Offset
    GUIList_ensure_visible(self);
}

int GUIList_get_current_page(GUIList* self) {
    if (self->visible_rows == 0) return 0;  // Divide by zero safety

    // Page = Selected Index / Items Per Page
    return (self->selected_index / self->visible_rows) + 1;
}

// --- Internal Draw Function ---
static void GUIList_draw(GUIComponent* self) {
    GUIList* list = (GUIList*)self;

    // 1. Safety Checks
    if (!list->delegate.get_item || !list->delegate.get_count) return;

    // 2. Get total items from delegate
    uint32_t total_items = list->delegate.get_count(list->delegate.context);

    // 3. Set Font
    // (Assuming you have an abstract wrapper, otherwise use u8g2 directly)
    // GUIRenderer_set_font(list->font_size);

    // 4. Iterate ONLY through visible rows
    for (int i = 0; i < list->visible_rows; i++) {
        // Calculate the actual data index
        uint32_t item_index = list->scroll_offset + i;

        // Stop if we ran out of data
        if (item_index >= total_items) break;

        // Get the string
        const char* text =
            list->delegate.get_item(list->delegate.context, item_index);
        if (!text) continue;

        // Calculate Y Position relative to the list component
        // x, y are uint8_t in your base component
        int draw_y = list->base.y + (i * list->row_height) +
                     list->row_height;  // +height for baseline
        int draw_x = list->base.x;

        // Draw Selection Indicator
        if (item_index == list->selected_index) {
            GUIRenderer_draw_str(draw_x, draw_y, ">");  // Cursor
            draw_x += 10;                               // Indent text
        }

        // Draw Text
        GUIRenderer_draw_str(draw_x, draw_y, text);
    }
}

// --- Constructor ---
GUIList* GUIList_new(GUIListDelegate delegate) {
    GUIList* list = (GUIList*)malloc(sizeof(GUIList));
    if (list) {
        GUIComponent_init(&list->base);

        // Wire up V-Table
        list->base.draw = GUIList_draw;
        // list->base.delete = GUIList_delete; // Don't forget to implement
        // delete!

        // Initialize State
        list->delegate = delegate;
        list->selected_index = 0;
        list->scroll_offset = 0;
        list->font_size = 8;     // Default
        list->visible_rows = 0;  // Will be calculated later
    }
    return list;  // FIXED: Added return statement
}

// --- Layout Calculation ---
void GUIList_recalculate_layout(GUIList* self) {
    // 1. Get Font Height (Abstracted or Direct)
    // Assuming 8px font + 2px padding = 10px
    uint8_t font_height = 8;
    uint8_t padding = 2;

    self->row_height = font_height + padding;

    // 2. Calculate how many rows fit in the component height
    if (self->row_height > 0) {
        self->visible_rows = self->base.height / self->row_height;
    } else {
        self->visible_rows = 0;
    }
}

// Inside your input handler (e.g., in GUIList_handle_input)
void GUIList_scroll_next_page(GUIList* self) {
    uint32_t total = self->delegate.get_count(self->delegate.context);

    // Jump offset by the number of visible rows
    if (self->scroll_offset + self->visible_rows < total) {
        self->scroll_offset += self->visible_rows;

        // Move selection to top of new page
        self->selected_index = self->scroll_offset;
    }
}