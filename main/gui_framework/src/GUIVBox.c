#include "Debug.h"
#include "GUIFramework.h"

// Private Layout Delegate
static void GUIVbox_update_layout(GUIComponent* base) {
    GUIVBox* self = (GUIVBox*)base;
    GUI_TRACE("GUIVbox_update_layout",
              "Updating layout of GUIVBox at address %p", self);

    if (self == NULL) return;

    const int x = self->base.base.x;
    const int y = self->base.base.y;
    const int height = self->base.base.height;
    const int width = self->base.base.width;
    const int padding = self->base.padding;
    const int spacing = self->base.spacing;
    const int children_count = self->base.count;

    if (children_count <= 0) return;

    // Calculate vertical space
    int available_height =
        height - (padding * 2) - (spacing * (children_count - 1));

    if (available_height < 0) available_height = 0;

    const int child_height = available_height / children_count;
    const int child_width = (width - padding * 2);

    const int height_remainder = available_height % children_count;

    for (int i = 0; i < children_count; i++) {
        GUIComponent* child = self->base.children[i];

        child->height = child_height;
        child->width = child_width;

        // Add remaining pixels to the last child
        if (i == children_count - 1) {
            child->height += height_remainder;
        }

        child->x = x + padding;
        // Stack vertically: y increases by child_height + spacing
        child->y = y + padding + i * (child_height + spacing);

        // Recursively update layout
        if (child->layout != NULL) {
            child->layout(child);
        }
    }
}

// Public Initializer
void GUIVBox_init(GUIVBox* self) {
    if (self == NULL) return;

    GUI_TRACE("GUIVBox_init", "Initializing GUIVBox at address %p", self);

    // Initialize the Base Container
    // Sets up children array, defaults, and registers the vertical layout
    // delegate
    GUIContainer_init(&self->base, GUIVbox_update_layout);
}