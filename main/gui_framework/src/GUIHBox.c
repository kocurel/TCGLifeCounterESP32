#include <GUIFramework.h>
#include <memory.h>
#include <stddef.h>

#include "Debug.h"
#include "GUIContainer.h"
struct GUIHBox {
    GUIContainer base;
};
static void GUIHBox_delete(GUIComponent* self) {
    GUI_TRACE("GUIHBox_delete", "Deleting GUIHBox at address %p", self);
    if (self == NULL) {
        return;
    }
    GUIHBox* hbox = (GUIHBox*)self;
    GUIContainer_delete(&hbox->base.base);
    free(hbox);
}

static void GUIHbox_update_layout(GUIComponent* base) {
    GUIHBox* self = (GUIHBox*)base;
    GUI_TRACE("GUIHbox_update_layout",
              "Updating layout of GUIHBox at address %p", self);
    if (self == NULL) {
        return;
    }
    int x = self->base.base.x;
    int y = self->base.base.y;
    int height = self->base.base.height;
    int width = self->base.base.width;
    int padding = self->base.padding;
    int spacing = self->base.spacing;
    int children_count = self->base.count;
    if (children_count <= 0) {
        return;
    }
    int child_height, child_width, available_width, width_remainder;
    child_height = (height - padding * 2);
    available_width = width - (padding * 2) - (spacing * (children_count - 1));
    child_width = available_width / children_count;

    width_remainder = available_width % children_count;

    for (int i = 0; i < children_count; i++) {
        GUIComponent* child = self->base.children[i];
        child->height = child_height;
        child->width = child_width;
        if (i == children_count - 1) {
            child->width += width_remainder;
        }
        child->y = y + padding;
        child->x = x + padding + i * (child_width + spacing);
        if (child->layout != NULL) {
            child->layout(child);
        }
    }
}

GUIHBox* GUIHBox_new() {
    GUI_TRACE("GUIHBox_new", "Creating a GUIHBox");
    GUIHBox* hbox = (GUIHBox*)malloc(sizeof(GUIHBox));
    GUI_TRACE("GUIHBox_new", "Created GUIHBox at address %p", hbox);
    GUIContainer_init(&hbox->base, GUIHbox_update_layout);
    hbox->base.base.delete = GUIHBox_delete;
    return hbox;
}
