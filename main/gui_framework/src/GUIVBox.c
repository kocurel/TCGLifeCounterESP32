#include <GUIFramework.h>
#include <memory.h>

#include "Debug.h"
#include "GUIContainer.h"

struct GUIVBox {
    GUIContainer base;
};
void GUIVBox_delete(GUIComponent* self) {
    GUI_TRACE("GUIVBox_delete", "Deleting GUIVBox at address %p", self);
    if (self == NULL) {
        return;
    }
    GUIVBox* vbox = (GUIVBox*)self;
    if (vbox == NULL) {
        return;
    }
    self->delete(&vbox->base.base);
    free(vbox);
}
static void GUIVbox_update_layout(GUIComponent* base) {
    GUIVBox* self = (GUIVBox*)base;
    GUI_TRACE("GUIVbox_update_layout",
              "Updating layout of GUIVBox at address %p", self);
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
    int child_height, child_width, available_height, height_remainder;
    available_height =
        height - (padding * 2) - (spacing * (children_count - 1));
    child_height = available_height / children_count;
    child_width = (width - padding * 2);

    height_remainder = available_height % children_count;

    for (int i = 0; i < children_count; i++) {
        GUIComponent* child = self->base.children[i];
        child->height = child_height;
        child->width = child_width;
        if (i == children_count - 1) {
            child->height += height_remainder;
        }
        child->x = x + padding;
        child->y = y + padding + i * (child_height + spacing);
        if (child->layout != NULL) {
            child->layout(child);
        }
    }
}
GUIVBox* GUIVBox_new() {
    GUI_TRACE("GUIVBox_new", "Creating a GUIVBox");
    GUIVBox* vbox = (GUIVBox*)malloc(sizeof(GUIVBox));
    GUI_TRACE("GUIVBox_new", "Created GUIVBox at address %p", vbox);
    GUIContainer_init(&vbox->base, GUIVbox_update_layout);
    vbox->base.base.delete = GUIVBox_delete;
    return vbox;
}