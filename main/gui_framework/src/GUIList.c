#include <math.h>

#include "GUIFramework.h"

#define LIST_ROW_HEIGHT 11

/**
 * Logika obliczająca płynny ruch kursora.
 */
void GUIList_tick(GUIList* self, uint32_t delta_ms) {
    // 1. Musimy wiedzieć, ile wierszy jest widocznych, żeby obliczyć target_y
    int visible_rows = self->base.height / LIST_ROW_HEIGHT;
    if (visible_rows < 1) visible_rows = 1;

    // 2. Obliczamy cel (relatywny wiersz) ZANIM wykonamy krok animacji
    int relative_selected_row = self->selected_index % visible_rows;
    self->target_y = self->base.y + (relative_selected_row * LIST_ROW_HEIGHT);

    // 3. Reszta logiki Lerp
    float base_speed = 0.25f;
    float time_factor = (float)delta_ms / 20.0f;
    float final_speed = base_speed * time_factor;
    if (final_speed > 0.9f) final_speed = 0.9f;

    self->anim_y += (self->target_y - self->anim_y) * final_speed;
}

static void GUIList_draw(GUIComponent* base) {
    GUIList* list = (GUIList*)base;
    if (list->get_count == NULL) return;
    int size = list->get_count(list->data_source);
    if (size == 0) return;

    int visible_rows = list->base.height / LIST_ROW_HEIGHT;
    if (visible_rows < 1) visible_rows = 1;
    int offset = (list->selected_index / visible_rows) * visible_rows;

    // Rysowanie wskaźnika (używa już obliczonego w ticku anim_y)
    uint8_t indicator_y = (uint8_t)(list->anim_y + 0.5f);
    GUIRenderer_set_font_size(7);
    GUIRenderer_draw_str(list->base.x, indicator_y + LIST_ROW_HEIGHT, ">");

    // 2. Rysowanie elementów listy
    for (int i = 0; i < visible_rows; i++) {
        int item_index = offset + i;
        if (item_index >= size) break;

        int x =
            list->base.x + 10;  // Przesunięcie tekstu, by nie nachodził na ">"
        int y = list->base.y + ((i + 1) * LIST_ROW_HEIGHT);

        if (list->draw_item != NULL) {
            // Jeśli mamy własny delegat rysowania (np. History Page)
            bool is_selected = (item_index == list->selected_index);
            list->draw_item(list, item_index, x, y, list->base.width - 10,
                            LIST_ROW_HEIGHT, is_selected);
        } else if (list->item_to_string != NULL) {
            // Domyślne rysowanie tekstu
            const char* str =
                list->item_to_string(list->data_source, item_index);
            GUIRenderer_draw_str(x, y, str);
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
    self->anim_y = 0;
    self->target_y = 0;
    self->needs_redraw = true;
}
void GUIList_up(GUIList* self) {
    if (self->selected_index > 0) {
        self->selected_index--;
        self->needs_redraw = true;  // Zmieniliśmy stan!
    }
}

void GUIList_down(GUIList* self) {
    if (self->selected_index < self->get_count(self->data_source) - 1) {
        self->selected_index++;
        self->needs_redraw = true;  // Zmieniliśmy stan!
    }
}
int GUIList_get_current_index(GUIList* self) { return self->selected_index; }