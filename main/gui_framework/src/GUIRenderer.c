#include "GUIRenderer.h"

#include "Debug.h"
#include "esp_log.h"
#include "u8g2.h"
#include "u8g2_esp32_hal.h"

// Static instance of the u8g2 structure
static u8g2_t u8g2_context;

// I2C Configuration (XIAO ESP32-C3)
// 0x3C shifted left by 1 is 0x78
#define I2C_DISPLAY_ADDR_SHIFTED 0x78
#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 7

#include "driver/i2c.h"
#include "esp_log.h"

void GUIRenderer_init() {
    // 1. Initialize the HAL structure
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.bus.i2c.sda = I2C_SDA_PIN;
    u8g2_esp32_hal.bus.i2c.scl = I2C_SCL_PIN;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    // Use the "Hardware I2C" constructor (starts with u8g2_Setup_...)
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2_context, U8G2_R0,
                                           u8g2_esp32_i2c_byte_cb,
                                           u8g2_esp32_gpio_and_delay_cb);

    // 3. Set the Address
    u8x8_SetI2CAddress(&u8g2_context.u8x8, I2C_DISPLAY_ADDR_SHIFTED);

    // 4. Wake up
    u8g2_InitDisplay(&u8g2_context);
    u8g2_SetPowerSave(&u8g2_context, 0);  // Wake up display
    u8g2_SetContrast(&u8g2_context, 32);

    u8g2_ClearBuffer(&u8g2_context);
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
    GUI_TRACE("GUIRenderer_set_contrast", "Setting OLED contrast to %u",
              contrast);
    u8g2_SetContrast(&u8g2_context, contrast);
}

void GUIRenderer_draw_horizontal_line(int y) {
    u8g2_DrawHLine(&u8g2_context, 0, y, 128);
}
void GUIRenderer_set_color(uint8_t color) {
    u8g2_SetDrawColor(&u8g2_context, color);
}

void GUIRenderer_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    u8g2_DrawLine(&u8g2_context, x1, y1, x2, y2);
}

void GUIRenderer_draw_pixel(uint8_t x, uint8_t y) {
    u8g2_DrawPixel(&u8g2_context, x, y);
}
void GUIRenderer_draw_xor_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    // 0 = Transparent, 1 = Solid, 2 = XOR
    u8g2_SetDrawColor(&u8g2_context, 2);

    // DrawBox fills the rectangle, applying the XOR operation
    u8g2_DrawBox(&u8g2_context, x, y, w, h);

    // Restore standard draw color
    u8g2_SetDrawColor(&u8g2_context, 1);
}