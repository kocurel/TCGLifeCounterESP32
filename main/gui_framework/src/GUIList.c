#include "GUIFramework.h"

#define LIST_ROW_HEIGHT 12
#define LIST_TEXT_PADDING 2

static void GUIList_draw(GUIComponent* base) {
    // 1. Cast Base to Specific
    GUIList* self = (GUIList*)base;

    // Safety: If no delegates are assigned, we can't draw anything
    if (self == NULL || self->get_count == NULL ||
        self->item_to_string == NULL) {
        return;
    }

    // 2. Get Data Count
    int size = self->get_count(self->data_source);
    if (size == 0) return;  // Nothing to draw

    // 3. Calculate Pagination
    // visible_rows: How many items fit in the component's height?
    int visible_rows = self->base.height / LIST_ROW_HEIGHT;
    if (visible_rows < 1) visible_rows = 1;  // Safety

    // offset: The index of the first item on the CURRENT page
    // This creates "Page Snapping" behavior.
    int offset = (self->selected_index / visible_rows) * visible_rows;

    // 4. Draw Loop
    for (int i = 0; i < visible_rows; i++) {
        int item_index = offset + i;

        // Stop if we run out of data items
        if (item_index >= size) break;

        // A. Retrieve Content using Delegates
        void* item = self->get_item(self->data_source, item_index);
        const char* display_string = self->item_to_string(item, item_index);

        // B. Calculate Screen Position
        // relative_y is 0 for the first item on the page, 1 for the second,
        // etc.
        int relative_y = i * LIST_ROW_HEIGHT;
        int x = self->base.x;
        int y = self->base.y + relative_y;

        // C. Draw Selection Indicator
        if (item_index == self->selected_index) {
            // Draw a box around the selected item
            GUIRenderer_draw_frame(x, y, self->base.width, LIST_ROW_HEIGHT);

            // Optional: You could fill the rect or invert colors here
            // GUIRenderer_fill_rect(x, y, self->base.width, LIST_ROW_HEIGHT);
        }

        // D. Draw Text
        // We add a little padding so text isn't stuck to the edge
        GUIRenderer_set_font_size(10);  // Ensure consistent font size
        GUIRenderer_draw_str(x + LIST_TEXT_PADDING, y + LIST_TEXT_PADDING,
                             display_string);
    }
}
void GUIList_init(GUIList* self, void* data_source,
                  int (*get_count)(void* data),
                  void* (*get_item)(void* data, int index),
                  char* (*item_to_string)(void* data, int index)) {
    GUIComponent_init(&self->base);
    self->base.draw = GUIList_draw;
    self->data_source = data_source;
    self->get_count = get_count;
    self->get_item = get_item;
    self->item_to_string = item_to_string;
    self->selected_index = 0;
}

void GUIList_draw(GUIComponent* self) {
    GUIList* list = (GUIList*)self;
    // draw;
}