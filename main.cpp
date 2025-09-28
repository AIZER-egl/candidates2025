#include "pico/stdlib.h"
#include <hardware/i2c.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>

#include "lib/hardware/oled.h"
#include "lib/hardware/button.h"

#define SDA 12
#define SCL 13
#define BOTON 15

int main() {
    stdio_init_all();
    gpio_init(BOTON);
    gpio_set_dir(BOTON, GPIO_IN);
    sleep_ms(2000);

    // Initialize I2C (i2c0) at 400kHz
    i2c_init(i2c0, 400000);
    gpio_pull_up(SDA);
    gpio_pull_up(SCL);
    gpio_set_function(SDA, GPIO_FUNC_I2C);
    gpio_set_function(SCL, GPIO_FUNC_I2C);

    // Configure and initialize the OLED
    ssd1306_t oled;
    oled.external_vcc = false;
    bool ok = ssd1306_begin(&oled, 128, 64, 0x3c, i2c0);

    std::vector<std::string> text_lines = {
        "Reto #1",
        "HOLA",
        "Reto #2",
        "Reto #69",
        "VdC (BSD)"
    };


    const int font_size = 1;
    const int line_height = 8; // Adjust if your font is taller

    int invert_line = 0; // index of the line to invert
    int y = invert_line * line_height;
    int text_width = text_lines[invert_line].length() * 6 * font_size; // 6 is typical font width
    int text_height = line_height;

    ssd1306_clear(&oled);
    int i = 0;
    for (const auto& line : text_lines) {
        if (i == invert_line) {
            ssd1306_draw_square(&oled, 0, y, text_width, text_height);
            ssd1306_clear_string(&oled, 0, y, font_size, line.c_str());
        } else {
            ssd1306_draw_string(&oled, 0, i * line_height, font_size, line.c_str());
        }
        i++;
    }
    ssd1306_show(&oled);

    // Main loop
    for (;;) {
        static bool prev_button_state = false;
        static bool move = true;

        bool button_state = gpio_get(BOTON);

        if (button_state && !prev_button_state && move) {
            invert_line = (invert_line + 1) % text_lines.size();
            y = invert_line * line_height;
            text_width = text_lines[invert_line].length() * 6 * font_size;

            ssd1306_clear(&oled);
            int i = 0;
            for (const auto& line : text_lines) {
            if (i == invert_line) {
                ssd1306_draw_square(&oled, 0, y, text_width, text_height);
                ssd1306_clear_string(&oled, 0, y, font_size, line.c_str());
            } else {
                ssd1306_draw_string(&oled, 0, i * line_height, font_size, line.c_str());
            }
            i++;
            }
            ssd1306_show(&oled);
            move = false;
        }

        if (!button_state && !move) {
            move = true;
        }

        prev_button_state = button_state;
    }

    return 0;
}
