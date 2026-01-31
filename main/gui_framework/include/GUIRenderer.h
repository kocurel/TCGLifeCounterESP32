#ifndef GUIRENDERER_H
#define GUIRENDERER_H

#include <stdint.h>

void GUIRenderer_init();
void GUIRenderer_draw_str(uint8_t x, uint8_t y, const char* text);
void GUIRenderer_draw_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void GUIRenderer_draw_frame(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void GUIRenderer_set_font_size(uint8_t font);

void GUIRenderer_rotate_text_enable();
void GUIRenderer_rotate_text_disable();

void GUIRenderer_clear_buffer();
void GUIRenderer_send_buffer();

void GUIRenderer_power_save_enable();
void GUIRenderer_power_save_disable();

uint8_t GUIRenderer_get_string_width(const char* str);

int8_t GUIRenderer_get_ascent();
int8_t GUIRenderer_get_descent();
void GUIRenderer_set_contrast(uint8_t contrast);
void GUIRenderer_draw_horizontal_line(int y);
void GUIRenderer_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void GUIRenderer_set_color(uint8_t color);
void GUIRenderer_draw_pixel(uint8_t x, uint8_t y);
#endif  // GUIRENDERER_H