#ifndef GUIFRAMEWORK_H
#define GUIFRAMEWORK_H

#include <stdbool.h>
#include <stdint.h>

#include "Debug.h"
#include "GUIRenderer.h"

typedef struct GUILabel GUILabel;
typedef struct GUIHBox GUIHBox;
typedef struct GUIVBox GUIVBox;
typedef struct GUIComponent GUIComponent;
typedef struct GUIContainer GUIContainer;

GUIVBox* GUIVBox_new();
GUIHBox* GUIHBox_new();
GUILabel* GUILabel_new();

void GUIComponent_set_pos(GUIComponent* self, uint8_t x, uint8_t y);
void GUIComponent_set_size(GUIComponent* self, uint8_t width, uint8_t height);
void GUIComponent_get_xywh(GUIComponent* self, uint8_t* x, uint8_t* y,
                           uint8_t* width, uint8_t* height);
void GUIComponent_draw(GUIComponent* self);
void GUIComponent_delete(GUIComponent* self);

void GUIContainer_set_padding(GUIContainer* self, int padding);
void GUIContainer_set_spacing(GUIContainer* self, int spacing);
void GUIContainer_add_child(GUIContainer* self, GUIComponent* child);
void GUIContainer_update_layout(GUIContainer* self);

void GUILabel_set_text(GUILabel* self, const char* str);
void GUILabel_set_font_size(GUILabel* self, uint8_t font);
void GUILabel_upside_down_en(GUILabel* self, int flag);

#define GUI_SET_SIZE(comp, w, h) \
    GUIComponent_set_size((GUIComponent*)(comp), (uint8_t)(w), (uint8_t)(h))

#define GUI_SET_POS(comp, x, y) \
    GUIComponent_set_pos((GUIComponent*)(comp), (uint8_t)(x), (uint8_t)(y))

#define GUI_ADD_CHILD(container, child) \
    GUIContainer_add_child((GUIContainer*)(container), (GUIComponent*)(child))

#define GUI_UPDATE_LAYOUT(container) \
    GUIContainer_update_layout((GUIContainer*)container)

#define GUI_DRAW(comp) GUIComponent_draw((GUIComponent*)comp)

#define GUI_SET_SPACING(container, spacing) \
    GUIContainer_set_spacing((GUIContainer*)container, (int)spacing)

#define GUI_SET_PADDING(container, padding) \
    GUIContainer_set_padding((GUIContainer*)container, (int)padding)
#define GUI_SET_TEXT(label, text) GUILabel_set_text((GUILabel*)label, text);

#define GUI_DELETE(comp) GUIComponent_delete((GUIComponent*)comp)

#define GUI_SET_FONT_SIZE(label, size) \
    GUILabel_set_font_size((GUILabel*)label, (uint8_t)size)

#define GUI_SET_TEXT_UPSIDE_DOWN(label, flag) \
    GUILabel_upside_down_en((GUILabel*)label, (int)flag)
#endif  // GUIFRAMEWORK_H