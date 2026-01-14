#include "GUIRenderer.h"

#include "Debug.h"
#include "u8g2.h"
#include "u8g2_esp32_hal.h"

static u8g2_t u8g2_context;

void GUIRenderer_init() {
    GUI_TRACE("GUIRenderer_init", "Initializing the u8g2 rendering context");
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    // Here you would set the pin numbers as per your hardware configuration
    u8g2_esp32_hal.bus.spi.clk = 8;    // Example pin number for CLK
    u8g2_esp32_hal.bus.spi.mosi = 10;  // Example pin number for MOSI
    u8g2_esp32_hal.bus.spi.cs = 9;     // Example pin number for CS
    u8g2_esp32_hal.dc = 7;             // Example pin number for DC
    u8g2_esp32_hal.reset = 21;         // Example pin number for RESET

    u8g2_esp32_hal_init(u8g2_esp32_hal);
    u8g2_Setup_ssd1309_128x64_noname0_f(&u8g2_context, U8G2_R0,
                                        u8g2_esp32_spi_byte_cb,
                                        u8g2_esp32_gpio_and_delay_cb);

    u8g2_InitDisplay(&u8g2_context);
    u8g2_SetPowerSave(&u8g2_context, 0);  // Wake up display
}

void GUIRenderer_draw_str(uint8_t x, uint8_t y, const char* text) {
    u8g2_DrawStr(&u8g2_context, x, y, text);
}

void GUIRenderer_draw_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    u8g2_DrawBox(&u8g2_context, x, y, w, h);
}
void GUIRenderer_draw_frame(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    u8g2_DrawFrame(&u8g2_context, x, y, w, h);
}

void GUIRenderer_set_font_size(uint8_t size) {
    GUI_TRACE("GUIRenderer_set_font_size", "Setting font size to to size %u",
              size);
    switch (size) {
        case 6:
            u8g2_SetFont(&u8g2_context, u8g2_font_5x7_mr);
            break;
        case 7:
            u8g2_SetFont(&u8g2_context, u8g2_font_6x10_mr);
            break;
        case 8:
            u8g2_SetFont(&u8g2_context, u8g2_font_6x12_mr);
            break;
        case 9:
            u8g2_SetFont(&u8g2_context, u8g2_font_6x13_mr);
            break;
        case 10:
            u8g2_SetFont(&u8g2_context, u8g2_font_7x14_mr);
            break;
        case 11:
            u8g2_SetFont(&u8g2_context, u8g2_font_8x13_mr);
            break;
        case 12:
            u8g2_SetFont(&u8g2_context, u8g2_font_9x15_mr);
            break;
        case 13:
            u8g2_SetFont(&u8g2_context, u8g2_font_9x18_mr);
            break;
        case 14:
            u8g2_SetFont(&u8g2_context, u8g2_font_10x20_mr);
            break;
    }
}
void GUIRenderer_rotate_text_enable() {
    u8g2_SetFontDirection(&u8g2_context, 2);
}
void GUIRenderer_rotate_text_disable() {
    u8g2_SetFontDirection(&u8g2_context, 0);
}

void GUIRenderer_clear_buffer() { u8g2_ClearBuffer(&u8g2_context); }
void GUIRenderer_send_buffer() { u8g2_SendBuffer(&u8g2_context); }

void GUIRenderer_power_save_enable() { u8g2_SetPowerSave(&u8g2_context, 1); }
void GUIRenderer_power_save_disable() { u8g2_SetPowerSave(&u8g2_context, 0); }
uint8_t GUIRenderer_get_string_width(const char* str) {
    return u8g2_GetStrWidth(&u8g2_context, str);
}
int8_t GUIRenderer_get_ascent() { return u8g2_GetAscent(&u8g2_context); }
int8_t GUIRenderer_get_descent() { return u8g2_GetDescent(&u8g2_context); }
void GUIRenderer_set_contrast(uint8_t contrast) {
    GUI_TRACE("GUIRenderer_set_contrast", "Setting OLED contrast");
    u8g2_SetContrast(&u8g2_context, contrast);
}

void GUIRenderer_draw_horizontal_line(int y) {
    u8g2_DrawHLine(&u8g2_context, 0, y, 128);
}
void GUIRenderer_set_color(uint8_t color) {
    u8g2_SetDrawColor(&u8g2_context, color);
}