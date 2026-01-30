#ifndef VALUEEDITORPAGE_H
#define VALUEEDITORPAGE_H
#include <stdint.h>

void ValueEditorPage_enter(const char* title, const char* subtitle,
                           uint8_t value_id, int32_t value,
                           void (*callback)(int32_t new_value));
#endif  // VALUEEDITORPAGE_H