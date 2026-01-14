#include <memory.h>
#include <stddef.h>

#include "Debug.h"
#include "GUIFramework.h"

static void GUIContainer_draw(GUIComponent* self) {
    GUI_TRACE("GUIContainer_draw", "Drawing GUIContainer at address %p", self);
    if (self == NULL) {
        return;
    }
    GUIContainer* parent = (GUIContainer*)self;
    for (int i = 0; i < parent->count; i++) {
        GUIComponent* child = parent->children[i];
        if (child != NULL && child->draw != NULL) {
            child->draw(child);
        }
    }
}
void GUIContainer_set_padding(GUIContainer* self, int padding) {
    if (self == NULL) {
        return;
    }
    self->padding = padding;
}

void GUIContainer_set_spacing(GUIContainer* self, int spacing) {
    if (self == NULL) {
        return;
    }
    self->spacing = spacing;
}

void GUIContainer_init(GUIContainer* self, void(layout)(GUIComponent* base)) {
    GUI_TRACE("GUIContainer_init", "Initializing GUIContainer at address %p",
              self);

    if (self == NULL) {
        return;
    }

    // 1. Initialize Base Component (sets base properties to 0, vtable to NULL)
    GUIComponent_init(&self->base);

    // 2. Assign V-Table
    self->base.layout = layout;
    self->base.draw = GUIContainer_draw;

    // 3. Initialize Container-Specific State
    self->count = 0;
    self->padding = 0;
    self->spacing = 0;

    // 4. MEMORY SAFETY FIX: Zero out the children array.
    memset(self->children, 0, sizeof(self->children));
}

void GUIContainer_add_child(GUIContainer* self, GUIComponent* child) {
    GUI_TRACE("GUIContainer_add_child",
              "Adding child at address %p to container at adress %p", child,
              self);
    if (self == NULL) {
        return;
    }
    if (self->count >= MAX_CONTAINER_CHILDREN) {
        GUI_TRACE("GUIContainer_add_child",
                  "ERROR: GUIContainer is already full %p", self);
        return;
    }
    self->children[self->count++] = child;
}

void GUIContainer_add_multiple(GUIContainer* self, void** children) {
    if (self == NULL || children == NULL) return;

    for (int i = 0; children[i] != NULL; i++) {
        GUIContainer_add_child(self, (GUIComponent*)children[i]);
    }
}

void GUIContainer_update_layout(GUIContainer* self) {
    GUI_TRACE("GUIContainer_update_layout",
              "Updating layout of GUIContainer at address %p", self);
    if (self == NULL) {
        return;
    }
    self->base.layout(&self->base);
    for (int i = 0; i < self->count; i++) {
        if (self->children[i]->layout != NULL) {
            self->children[i]->layout(self->children[i]);
        }
    }
}
