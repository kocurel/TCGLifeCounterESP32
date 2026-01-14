#include "GUIFramework.h"

#define LIST_ROW_HEIGHT 11

static void GUIList_draw(GUIComponent* base) {
    GUI_TRACE("GUIList_draw", "Drawing GUIList at address %p", base);
    // 1. Cast Base to Specific
    GUIList* list = (GUIList*)base;

    if (list->get_count == NULL ||
        (list->item_to_string == NULL && list->draw_item == NULL)) {
        return;
    }

    int size = list->get_count(list->data_source);
    GUI_TRACE("GUIList_draw", "Element count: %d", size);

    if (size == 0) {
        printf("empty list\n");
        return;
    }

    int visible_rows = list->base.height / LIST_ROW_HEIGHT;
    if (visible_rows < 1) visible_rows = 1;  // Safety
    GUI_TRACE("GUIList_draw", "Visible rows: %d", visible_rows);

    int offset = (list->selected_index / visible_rows) * visible_rows;

    for (int i = 0; i < visible_rows; i++) {
        int item_index = offset + i;

        if (item_index >= size) break;
        GUI_TRACE("GUIList_draw", "Drawing change with index %d", item_index);
        // B. Calculate Screen Position
        // relative_y is 0 for the first item on the page, 1 for the second,
        // etc.
        int relative_y = (i + 1) * LIST_ROW_HEIGHT;
        int x = list->base.x;
        int y = list->base.y + relative_y;

        GUIRenderer_set_font_size(7);
        bool is_selected = item_index == list->selected_index;
        if (list->draw_item != NULL) {
            // OPTION A: Custom Delegate (For History Page)
            // The delegate handles everything: selection, text, layout
            list->draw_item(list, item_index, x, y, list->base.width,
                            LIST_ROW_HEIGHT, is_selected);
        } else {
            // OPTION B: Default String Behavior (For Menu Page)
            void* item = NULL;
            if (list->get_item) {
                item = list->get_item(list->data_source, item_index);
            }

            // Only try to convert string if the function exists
            if (list->item_to_string) {
                const char* display_string =
                    list->item_to_string(item, item_index);

                GUIRenderer_set_font_size(7);

                // Default Selection Indicator
                if (is_selected) {
                    GUIRenderer_draw_str(x, y, ">");
                    x += 8;
                }
                GUIRenderer_draw_str(x, y, display_string);
            }
        }
    }
}
void GUIList_init(GUIList* self, void* data_source,
                  int (*get_count)(void* data),
                  void* (*get_item)(void* data, int index),
                  char* (*item_to_string)(void* data, int index),
                  void (*draw_item)(struct GUIList* list, int index, uint8_t x,
                                    uint8_t y, uint8_t width, uint8_t height,
                                    bool is_selected)) {
    GUIComponent_init(&self->base);
    self->base.draw = GUIList_draw;
    self->data_source = data_source;
    self->get_count = get_count;
    self->get_item = get_item;
    self->item_to_string = item_to_string;
    self->draw_item = draw_item;
    self->selected_index = 0;
}

void GUIList_up(GUIList* self) {
    self->selected_index--;
    if (self->selected_index < 0) {
        self->selected_index = 0;
    }
}

void GUIList_down(GUIList* self) {
    self->selected_index++;
    if (self->selected_index >= self->get_count(self->data_source))
        self->selected_index = self->get_count(self->data_source) - 1;
}

int GUIList_get_current_index(GUIList* self) { return self->selected_index; }