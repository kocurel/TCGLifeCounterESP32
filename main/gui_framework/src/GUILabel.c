#include <stdio.h>  // Required for snprintf

#include "GUIFramework.h"

static void GUILabel_draw(GUIComponent* self) {
    if (self == NULL) return;

    GUILabel* label = (GUILabel*)self;

    GUI_TRACE("GUILabel_draw", "Drawing GUILabel at address %p", self);

    GUIRenderer_set_font_size(label->font_size);

    uint8_t text_width = GUIRenderer_get_string_width(label->text) - 1;

    int8_t ascent = GUIRenderer_get_ascent();
    int8_t descent = GUIRenderer_get_descent();
    int8_t text_height = ascent - descent;

    // 4. Draw Normal or Rotated
    if (!label->isUpsideDown) {
        GUIRenderer_rotate_text_disable();
        uint8_t x, y;

        y = self->y + ascent + (self->height - text_height) / 2;

        switch (label->alignment) {
            case GUI_ALIGMNENT_CENTER:
                x = self->x + (self->width - text_width) / 2;
                break;
            case GUI_ALIGMNENT_LEFT:
                x = self->x;
                break;
            case GUI_ALIGMNENT_RIGHT:
                x = self->x + self->width - text_width;
                break;
            default:
                x = 0;
                break;
        }
        GUIRenderer_draw_str(x, y, label->text);
    } else {
        GUIRenderer_rotate_text_enable();

        uint8_t y = self->y - GUIRenderer_get_descent() +
                    (self->height - text_height) / 2;

        uint8_t x;
        switch (label->alignment) {
            case GUI_ALIGMNENT_CENTER:
                // (W - w)/2 + w  ==> (W + w)/2
                x = self->x + (self->width + text_width) / 2;
                break;
            case GUI_ALIGMNENT_LEFT:
                // Wizualnie lewa, więc fizycznie prawa krawędź tekstu dotyka
                // prawej krawędzi boksu Ale że rysujemy "od tyłu" (zależy od
                // biblioteki), często to jest:
                x = self->x + self->width;
                break;
            case GUI_ALIGMNENT_RIGHT:
                // Wizualnie prawa
                x = self->x + text_width;
                break;
            default:
                x = self->x + text_width;
                break;
        }
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
    self->alignment = GUI_ALIGMNENT_CENTER;

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
void GUILabel_set_alignment(GUILabel* self, uint8_t alignment) {
    if (self == NULL) return;
    self->alignment = alignment;
}
