#include <math.h>

#include "GUIFramework.h"

#define LIST_ROW_HEIGHT 11
#define LERP_BASE_SPEED 0.25f
#define LERP_TIME_STEP 20.0f
#define LERP_MAX_SPEED 0.9f
#define LERP_THRESHOLD 0.1f

/**
 * Updates smooth cursor movement using adaptive LERP.
 */
void GUIList_tick(GUIList* self, uint32_t delta_ms) {
    if (self == NULL) return;

    const int visible_rows = self->base.height / LIST_ROW_HEIGHT;
    const int eff_visible = (visible_rows < 1) ? 1 : visible_rows;

    // Calculate vertical target based on selected index paging
    const int rel_row = self->selected_index % eff_visible;
    self->target_y = (float)(self->base.y + (rel_row * LIST_ROW_HEIGHT));

    const float time_factor = (float)delta_ms / LERP_TIME_STEP;
    float final_speed = LERP_BASE_SPEED * time_factor;
    if (final_speed > LERP_MAX_SPEED) final_speed = LERP_MAX_SPEED;

    const float diff = self->target_y - self->anim_y;

    // Snap to target if within threshold to stop floating point drift
    if (fabsf(diff) < LERP_THRESHOLD) {
        self->anim_y = self->target_y;
    } else {
        self->anim_y += diff * final_speed;
    }
}

static void GUIList_draw(GUIComponent* base) {
    GUIList* const list = (GUIList*)base;
    if (list == NULL || list->get_count == NULL) return;

    const int size = list->get_count(list->data_source);
    if (size == 0) return;

    const int visible_rows = list->base.height / LIST_ROW_HEIGHT;
    const int eff_visible = (visible_rows < 1) ? 1 : visible_rows;

    // Calculate page offset
    const int offset = (list->selected_index / eff_visible) * eff_visible;

    // Render selection indicator using animated position
    const uint8_t indicator_y = (uint8_t)(list->anim_y + 0.5f);
    GUIRenderer_set_font_size(7);
    GUIRenderer_draw_str(list->base.x, indicator_y + LIST_ROW_HEIGHT, ">");

    // Render list items for current page
    for (int i = 0; i < eff_visible; i++) {
        const int item_index = offset + i;
        if (item_index >= size) break;

        const int x = list->base.x + 10;
        const int y = list->base.y + ((i + 1) * LIST_ROW_HEIGHT);

        if (list->draw_item != NULL) {
            // Custom item delegate
            const bool is_selected = (item_index == list->selected_index);
            list->draw_item(list, item_index, (uint8_t)x, (uint8_t)y,
                            (uint8_t)(list->base.width - 10),
                            (uint8_t)LIST_ROW_HEIGHT, is_selected);
        } else if (list->item_to_string != NULL) {
            // Default string rendering
            const char* const str =
                list->item_to_string(list->data_source, item_index);
            if (str != NULL) GUIRenderer_draw_str(x, y, str);
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
    if (self && self->selected_index > 0) {
        self->selected_index--;
        self->needs_redraw = true;
    }
}

void GUIList_down(GUIList* self) {
    if (self && self->selected_index < self->get_count(self->data_source) - 1) {
        self->selected_index++;
        self->needs_redraw = true;
    }
}

int GUIList_get_current_index(GUIList* self) {
    return self ? self->selected_index : -1;
}