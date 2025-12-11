#ifndef GUICONTAINER_H
#define GUICONTAINER_H

#include "GUIComponent.h"
#include "GUIFramework.h"
#define MAX_CONTAINER_CHILDREN 8

struct GUIContainer {
    GUIComponent base;
    void (*add_child)(GUIContainer* self, GUIComponent* child);
    GUIComponent* children[MAX_CONTAINER_CHILDREN];
    int count;
    int spacing;
    int padding;
};

void GUIContainer_init(GUIContainer* self, void(layout)(GUIComponent* base));
void GUIContainer_delete(GUIContainer* self);
#endif  // GUICONTAINER_H
