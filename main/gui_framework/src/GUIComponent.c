#include <stddef.h>

#include "Debug.h"
#include "GUIFramework.h"

void GUIComponent_init(GUIComponent* self) {
    GUI_TRACE("GUIComponent_init", "Initiating GUIComponent at address %p",
              self);
    if (self == NULL) {
        return;
    }
    self->x = 0;
    self->y = 0;
    self->width = 0;
    self->height = 0;
    self->draw = NULL;
    self->layout = NULL;

    self->nav_up = NULL;
    self->nav_down = NULL;
    self->nav_left = NULL;
    self->nav_right = NULL;
}

void GUIComponent_set_pos(GUIComponent* self, uint8_t x, uint8_t y) {
    if (self == NULL) {
        return;
    }
    self->x = x;
    self->y = y;
}

void GUIComponent_set_size(GUIComponent* self, uint8_t width, uint8_t height) {
    if (self == NULL) {
        return;
    }
    self->width = width;
    self->height = height;
}
void GUIComponent_get_xywh(GUIComponent* self, uint8_t* x, uint8_t* y,
                           uint8_t* width, uint8_t* height) {
    *x = self->x;
    *y = self->y;
    *width = self->width;
    *height = self->height;
}
void GUIComponent_draw(GUIComponent* self) {
    GUI_TRACE("GUIComponent_draw", "Drawing GUIComponent at address %p", self);

    if (self != NULL && self->draw != NULL) {
        self->draw(self);
    }
}