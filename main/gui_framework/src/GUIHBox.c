#include "Debug.h"
#include "GUIContainer.h"
#include "GUIFramework.h"

// --- Private Layout Delegate ---
// Kept static so it remains internal to this file
static void GUIHbox_update_layout(GUIComponent* base) {
    GUIHBox* self = (GUIHBox*)base;
    GUI_TRACE("GUIHbox_update_layout",
              "Updating layout of GUIHBox at address %p", self);

    if (self == NULL) return;

    // Accessing base properties through the inheritance chain
    // HBox -> Container -> Component
    int x = self->base.base.x;
    int y = self->base.base.y;
    int height = self->base.base.height;
    int width = self->base.base.width;
    int padding = self->base.padding;
    int spacing = self->base.spacing;
    int children_count = self->base.count;

    if (children_count <= 0) return;

    int child_height, child_width, available_width, width_remainder;

    child_height = (height - padding * 2);
    available_width = width - (padding * 2) - (spacing * (children_count - 1));

    // Safety check to prevent division by zero or negative width
    if (available_width < 0) available_width = 0;

    child_width = available_width / children_count;
    width_remainder = available_width % children_count;

    for (int i = 0; i < children_count; i++) {
        GUIComponent* child = self->base.children[i];

        child->height = child_height;
        child->width = child_width;

        // Add remainder pixels to the last child to fill space perfectly
        if (i == children_count - 1) {
            child->width += width_remainder;
        }

        child->y = y + padding;
        child->x = x + padding + i * (child_width + spacing);

        // Recursively update layout of children
        if (child->layout != NULL) {
            child->layout(child);
        }
    }
}

// --- Public Initializer ---
// Replaces GUIHBox_new
void GUIHBox_init(GUIHBox* self) {
    if (self == NULL) return;

    GUI_TRACE("GUIHBox_init", "Initializing GUIHBox at address %p", self);

    // Initialize the Base Container
    // This sets up the children array, defaults (padding=0, spacing=0),
    // and registers the specific layout function 'GUIHbox_update_layout'
    GUIContainer_init(&self->base, GUIHbox_update_layout);
}