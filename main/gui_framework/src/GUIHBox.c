#include "Debug.h"
#include "GUIFramework.h"

// Private Layout Delegate
static void GUIHbox_update_layout(GUIComponent* base) {
    GUIHBox* self = (GUIHBox*)base;
    GUI_TRACE("GUIHbox_update_layout",
              "Updating layout of GUIHBox at address %p", self);

    if (self == NULL) return;

    // Accessing base properties through the inheritance chain
    // HBox -> Container -> Component
    const int x = self->base.base.x;
    const int y = self->base.base.y;
    const int height = self->base.base.height;
    const int width = self->base.base.width;
    const int padding = self->base.padding;
    const int spacing = self->base.spacing;
    int children_count = self->base.count;

    if (children_count <= 0) return;

    const int child_height = (height - padding * 2);

    int available_width =
        width - (padding * 2) - (spacing * (children_count - 1));

    // Safety check to prevent division by zero or negative width
    if (available_width < 0) available_width = 0;

    const int child_width = available_width / children_count;
    const int width_remainder = available_width % children_count;

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
void GUIHBox_init(GUIHBox* self) {
    if (self == NULL) return;

    GUI_TRACE("GUIHBox_init", "Initializing GUIHBox at address %p", self);

    // Initialize the Base Container
    // Sets up children array, defaults, and registers the vertical layout
    // delegate
    GUIContainer_init(&self->base, GUIHbox_update_layout);
}