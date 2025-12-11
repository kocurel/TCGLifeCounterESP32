#include "GUIContainer.h"
#include "GUIFramework.h"

struct GUIVBox {
    GUIContainer base;
};
struct GUIHBox {
    GUIContainer base;
};
struct GUILabel {
    GUIComponent base;
    char* text;
    uint8_t font_size;
    bool isUpsideDown;
};
