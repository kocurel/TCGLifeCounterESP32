#ifndef KEYBOARD_PAGE_H
#define KEYBOARD_PAGE_H

typedef void (*KeyboardCallback)(const char* result);

void KeyboardPage_enter(const char* title, const char* initial_text,
                        KeyboardCallback callback);

#endif  // KEYBOARD_PAGE_H