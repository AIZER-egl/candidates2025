#include "pico/stdlib.h"
#include <hardware/i2c.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>

#include "lib/hardware/oled.h"
#include "lib/hardware/Button/button.h"

#define SDA 12
#define SCL 13

void update_display(ssd1306_t* oled, const std::vector<std::string>& text_lines, int selected_line) {
    const int font_size = 1;
    const int line_height = 8;
    
    ssd1306_clear(oled);
    
    for (int i = 0; i < text_lines.size(); i++) {
        int y = i * line_height;
        
        if (i == selected_line) {
            // Draw inverted line (selected)
            int text_width = text_lines[i].length() * 6 * font_size;
            ssd1306_draw_square(oled, 0, y, text_width, line_height);
            ssd1306_clear_string(oled, 0, y, font_size, text_lines[i].c_str());
        } else {
            // Draw normal line
            ssd1306_draw_string(oled, 0, y, font_size, text_lines[i].c_str());
        }
    }
    
    ssd1306_show(oled);
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    // Initialize I2C (i2c0) at 400kHz
    i2c_init(i2c0, 400000);
    gpio_pull_up(SDA);
    gpio_pull_up(SCL);
    gpio_set_function(SDA, GPIO_FUNC_I2C);
    gpio_set_function(SCL, GPIO_FUNC_I2C);

    // Initialize button
    ActionButton actionButton;
    actionButton.begin();

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

    int selected_line = 0;
    
    // Initial display update
    update_display(&oled, text_lines, selected_line);

    // Main loop
    for (;;) {
        actionButton.tick();
        
        State button_state = actionButton.getState();
        
        if (button_state == PRESSED) {
            std::cout << "Button Pressed" << std::endl;
        } else if (button_state == HOLD) {
            std::cout << "Button hold" << std::endl;
        }
    }

    return 0;
}