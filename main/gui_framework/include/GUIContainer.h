#ifndef GUICONTAINER_H
#define GUICONTAINER_H

#include "GUIFramework.h"

void GUIContainer_init(GUIContainer* self, void(layout)(GUIComponent* base));
void GUIContainer_delete(GUIContainer* self);
#endif  // GUICONTAINER_H
