#ifndef OLED_H
#define OLED_H

#define OLED_ADDRESS 0x3c
#define OLED_I2C_INSTANCE i2c0
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#include "../software/font.h"
#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <utility>

class Oled {
    public:
        Oled();
        bool begin();
        void tick();

        struct UIElement {
            std::string label;
            int id;
            bool active;
        };

        typedef enum {
            SET_CONTRAST = 0x81,
            SET_ENTIRE_ON = 0xA4,
            SET_NORM_INV = 0xA6,
            SET_DISP = 0xAE,
            SET_MEM_ADDR = 0x20,
            SET_COL_ADDR = 0x21,
            SET_PAGE_ADDR = 0x22,
            SET_DISP_START_LINE = 0x40,
            SET_SEG_REMAP = 0xA0,
            SET_MUX_RATIO = 0xA8,
            SET_COM_OUT_DIR = 0xC0,
            SET_DISP_OFFSET = 0xD3,
            SET_COM_PIN_CFG = 0xDA,
            SET_DISP_CLK_DIV = 0xD5,
            SET_PRECHARGE = 0xD9,
            SET_VCOM_DESEL = 0xDB,
            SET_CHARGE_PUMP = 0x8D
        } ssd1306_command_t;

        void draw_string(uint32_t x, uint32_t y, uint32_t scale, const char *s, bool inverse = false);
    private:
        i2c_inst_t* i2c_instance;
        uint8_t address;
        uint8_t pages;		/**< stores pages of display (calculated on initialization*/
        uint8_t* buffer;	/**< display buffer */
        uint16_t width;
        uint16_t height;
        int bufsize;		/**< buffer size */
        bool external_vcc; 	/**< whether display uses external vcc */ 
        const uint8_t* font;


        void show();
        void clear();
        void write(const uint8_t *src, size_t length);

        // Drawing
        void draw_pixel(uint32_t x, uint32_t y, bool inverse = false);
        void draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
        void draw_square(uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool inverse = false);
        void draw_char(uint32_t x, uint32_t y, uint32_t scale, char c, bool inverse = false);
};

#endif // OLED_H