#ifndef GUICOMPONENT_H
#define GUICOMPONENT_H
#include <stdint.h>

#include "GUIFramework.h"

struct GUIComponent {
    void (*draw)(GUIComponent* self);
    void (*layout)(GUIComponent* self);
    void (*delete)(GUIComponent* self);
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
};
void GUIComponent_init(GUIComponent* self);
void GUIComponent_delete(GUIComponent* self);

#endif  // GUICOMPONENT_H