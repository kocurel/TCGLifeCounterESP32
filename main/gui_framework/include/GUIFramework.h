#ifndef GUIFRAMEWORK_H
#define GUIFRAMEWORK_H

#include <stdbool.h>
#include <stdint.h>

#include "Debug.h"
#include "GUIRenderer.h"

// Finalized GUI components
typedef struct GUILabel GUILabel;
typedef struct GUIHBox GUIHBox;
typedef struct GUIVBox GUIVBox;
typedef struct GUIList GUIList;

// Abstract base GUI components
typedef struct GUIComponent GUIComponent;
typedef struct GUIContainer GUIContainer;

// constants
#define MAX_CONTAINER_CHILDREN 8
#define LABEL_MAX_SIZE 32

struct GUIComponent {
    void (*draw)(GUIComponent* self);
    void (*layout)(GUIComponent* self);
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    struct GUIComponent* nav_up;
    struct GUIComponent* nav_down;
    struct GUIComponent* nav_left;
    struct GUIComponent* nav_right;
};

struct GUIContainer {
    GUIComponent base;
    void (*add_child)(GUIContainer* self, GUIComponent* child);
    GUIComponent* children[MAX_CONTAINER_CHILDREN];
    int count;
    int spacing;
    int padding;
};

struct GUIHBox {
    GUIContainer base;
};

struct GUIVBox {
    GUIContainer base;
};

struct GUILabel {
    GUIComponent base;
    char text[LABEL_MAX_SIZE];
    uint8_t font_size;
    bool isUpsideDown;
};

struct GUIList {
    GUIComponent base;
    void* data_source;
    int (*get_count)(void* data);
    void* (*get_item)(void* data, int index);
    char* (*item_to_string)(void* data, int index);
    int selected_index;
};

void GUIHBox_init(GUIHBox* self);
void GUIVBox_init(GUIVBox* self);
void GUILabel_init(GUILabel* self, const char* text);
void GUIComponent_init(GUIComponent* self);
void GUIContainer_init(GUIContainer* self, void(layout)(GUIComponent* base));

void GUIComponent_set_pos(GUIComponent* self, uint8_t x, uint8_t y);
void GUIComponent_set_size(GUIComponent* self, uint8_t width, uint8_t height);
void GUIComponent_get_xywh(GUIComponent* self, uint8_t* x, uint8_t* y,
                           uint8_t* width, uint8_t* height);
void GUIComponent_draw(GUIComponent* self);

void GUIContainer_set_padding(GUIContainer* self, int padding);
void GUIContainer_set_spacing(GUIContainer* self, int spacing);
void GUIContainer_add_child(GUIContainer* self, GUIComponent* child);
void GUIContainer_add_multiple(GUIContainer* self, GUIComponent** children);
void GUIContainer_update_layout(GUIContainer* self);

void GUILabel_set_text(GUILabel* self, const char* str);
void GUILabel_set_font_size(GUILabel* self, uint8_t font);
void GUILabel_upside_down_en(GUILabel* self, bool flag);

void GUIList_up(GUIList* self);
void GUIList_down(GUIList* down);
void GUIList_init(GUIList* self, void* data_source,
                  int (*get_count)(void* data),
                  void* (*get_item)(void* data, int index),
                  char* (*item_to_string)(void* data, int index));

#define GUI_SET_SIZE(comp, w, h) \
    GUIComponent_set_size((GUIComponent*)(comp), (uint8_t)(w), (uint8_t)(h))

#define GUI_SET_POS(comp, x, y) \
    GUIComponent_set_pos((GUIComponent*)(comp), (uint8_t)(x), (uint8_t)(y))

#define GUI_ADD_CHILD(container, child) \
    GUIContainer_add_child((GUIContainer*)(container), (GUIComponent*)(child))

#define GUI_ADD_CHILDREN(container, ...)                  \
    GUIContainer_add_multiple((GUIContainer*)(container), \
                              (GUIComponent*[]){__VA_ARGS__, NULL})

#define GUI_UPDATE_LAYOUT(container) \
    GUIContainer_update_layout((GUIContainer*)container)

#define GUI_DRAW(comp) GUIComponent_draw((GUIComponent*)comp)

#define GUI_SET_SPACING(container, spacing) \
    GUIContainer_set_spacing((GUIContainer*)container, (int)spacing)

#define GUI_SET_PADDING(container, padding) \
    GUIContainer_set_padding((GUIContainer*)container, (int)padding)
#define GUI_SET_TEXT(label, text) GUILabel_set_text((GUILabel*)label, text);

#define GUI_SET_FONT_SIZE(label, size) \
    GUILabel_set_font_size((GUILabel*)label, (uint8_t)size)

#define GUI_SET_TEXT_UPSIDE_DOWN(label, flag) \
    GUILabel_upside_down_en((GUILabel*)label, (bool)flag)

// Helper macro to link two components horizontally
// (Bidirectional) Usage: GUI_LINK_H(&left_item,
// &right_item);
#define GUI_LINK_HORIZONTAL(left, right)                         \
    ((GUIComponent*)(left))->nav_right = (GUIComponent*)(right); \
    ((GUIComponent*)(right))->nav_left = (GUIComponent*)(left)

// Helper macro to link two components vertically
// (Bidirectional) Usage: GUI_LINK_V(&top_item,
// &bottom_item);
#define GUI_LINK_VERTICAL(top, bottom)                          \
    ((GUIComponent*)(top))->nav_down = (GUIComponent*)(bottom); \
    ((GUIComponent*)(bottom))->nav_up = (GUIComponent*)(top)
#endif  // GUIFRAMEWORK_H