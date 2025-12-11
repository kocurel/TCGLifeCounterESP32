#include <memory.h>

#include "Debug.h"
#include "GUIComponent.h"
#include "GUIFramework.h"
#include "GUIRenderer.h"

struct GUILabel {
    GUIComponent base;
    char* text;
    uint8_t font_size;
    bool isUpsideDown;
};

static void GUILabel_delete(GUIComponent* self) {
    GUI_TRACE("GUILabel_delete", "Deleting GUILabel at address %p", self);
    if (self == NULL) {
        return;
    }
    GUILabel* label = (GUILabel*)self;
    if (label == NULL) {
        return;
    }
    free(label->text);
    free(label);
}

static void GUILabel_draw(GUIComponent* self) {
    GUI_TRACE("GUILabel_draw", "Drawing GUILabel at address %p", self);
    if (self == NULL) {
        return;
    }
    GUILabel* label = (GUILabel*)self;
    GUIRenderer_set_font_size(label->font_size);
    uint8_t text_width = GUIRenderer_get_string_width(label->text);
    int8_t ascent = GUIRenderer_get_ascent();
    int8_t descent = GUIRenderer_get_descent();  // negative value
    int8_t text_height = ascent - descent;
    if (!label->isUpsideDown) {
        GUIRenderer_rotate_text_disable();
        uint8_t y = self->y + text_height + (self->height - text_height) / 2;
        uint8_t x = self->x + (self->width - text_width) / 2;
        GUIRenderer_draw_str(x, y, label->text);
    }
    // if text is rotated we need to move the (x,y) to what would have been the
    // upper right corner (text goes right to left)
    if (label->isUpsideDown) {
        GUIRenderer_rotate_text_enable();
        uint8_t x = self->x + (self->width - text_width) / 2 + text_width;
        uint8_t y = self->y + (self->height - text_height) / 2;
        GUIRenderer_draw_str(x, y, label->text);
    }
}

GUILabel* GUILabel_new() {
    GUI_TRACE("GUILabel_new", "Creating a GUILabel");
    GUILabel* label = (GUILabel*)malloc(sizeof(GUILabel));
    GUIComponent_init(&label->base);
    label->base.delete = GUILabel_delete;
    label->base.draw = GUILabel_draw;
    label->base.layout = NULL;
    label->text = strdup("");
    label->font_size = 10;
    label->isUpsideDown = false;
    GUI_TRACE("GUILabel_new", "Created GUILabel at address %p", label);
    return label;
}
void GUILabel_set_text(GUILabel* self, const char* str) {
    if (self == NULL) {
        return;
    }
    self->text = strdup(str);
}

void GUILabel_set_font_size(GUILabel* self, uint8_t font_size) {
    if (self == NULL) {
        return;
    }
    self->font_size = font_size;
}

void GUILabel_upside_down_en(GUILabel* self, int flag) {
    if (self == NULL) {
        return;
    }
    if (flag == 1) {
        self->isUpsideDown = true;
    } else if (flag == 0) {
        self->isUpsideDown = false;
    }
}
