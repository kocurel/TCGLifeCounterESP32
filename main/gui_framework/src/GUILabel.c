#include <stdio.h>  // Required for snprintf

#include "GUIFramework.h"

static void GUILabel_draw(GUIComponent* self) {
    if (self == NULL) return;

    GUILabel* label = (GUILabel*)self;

    GUI_TRACE("GUILabel_draw", "Drawing GUILabel at address %p", self);

    GUIRenderer_set_font_size(label->font_size);

    // subtracting a magic 1 cause string width calculation sucks
    uint8_t text_width = GUIRenderer_get_string_width(label->text) - 1;

    int8_t ascent = GUIRenderer_get_ascent();
    int8_t descent = GUIRenderer_get_descent();
    int8_t text_height = ascent - descent;

    // 4. Draw Normal or Rotated
    if (!label->isUpsideDown) {
        GUIRenderer_rotate_text_disable();
        // Center text in the bounding box
        printf("%u\n", text_width);
        uint8_t x = self->x + (self->width - text_width) / 2;
        uint8_t y = self->y + text_height + (self->height - text_height) / 2;
        GUI_TRACE(
            "GUILabel_draw",
            "GUILabel dimensions: x:%u y%u w%u h%u\nText at x:%u y:%u w%u h%u",
            self->x, self->y, self->width, self->height, x, y, text_width,
            text_height);
        GUIRenderer_draw_str(x, y, label->text);
    } else {
        GUIRenderer_rotate_text_enable();
        // Calculate rotated origin
        uint8_t x = self->x + (self->width - text_width) / 2 + text_width;
        uint8_t y = self->y + (self->height - text_height) / 2;
        GUIRenderer_draw_str(x, y, label->text);
    }
}

void GUILabel_init(GUILabel* self, const char* initial_text) {
    if (self == NULL) return;
    GUI_TRACE("GUILabel_init", "Initializing GUILabel at address %p", self);
    GUIComponent_init(&self->base);

    self->base.draw = GUILabel_draw;

    self->font_size = 10;
    self->isUpsideDown = false;

    if (initial_text) {
        GUILabel_set_text(self, initial_text);
    } else {
        self->text[0] = '\0';
    }
}

void GUILabel_set_text(GUILabel* self, const char* str) {
    if (self == NULL || str == NULL) return;
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