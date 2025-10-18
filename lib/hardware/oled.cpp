#include "oled.h"

Oled::Oled() {
    address = OLED_ADDRESS;
    i2c_instance = OLED_I2C_INSTANCE;
    width = OLED_WIDTH;
    height = OLED_HEIGHT;
    font = font_8x5;
    pages = height / 8;
    bufsize = pages * width;
    external_vcc = false;
};

bool Oled::begin() {
    std::cout << "Begin function started" << std::endl;

    buffer.resize(pages * width);

    std::vector<uint8_t> cmds = {
        SET_DISP,
        SET_DISP_CLK_DIV, 0x80,
        SET_MUX_RATIO, height - 1,
        SET_DISP_OFFSET, 0x00,
        SET_DISP_START_LINE,
        SET_CHARGE_PUMP, external_vcc ? 0x10 : 0x14,
        SET_SEG_REMAP | 0x01,
        SET_COM_OUT_DIR | 0x08,
        SET_COM_PIN_CFG, width > 2 * height ? 0x02 : 0x12,
        SET_CONTRAST, 0xff,
        SET_PRECHARGE, external_vcc ? 0x22 : 0xF1,
        SET_VCOM_DESEL, 0x30,
        SET_ENTIRE_ON,
        SET_NORM_INV,
        SET_DISP | 0x01,
        SET_MEM_ADDR, 0x00,
    };

    for (const unsigned char & cmd : cmds) {
        uint8_t src[2] = {0x00, cmd};
        i2c_write(src, 2);
    }

    return true;
}

void Oled::tick() {
    if (millis() - previous_show >= 500) {
        show();
    }
}

void Oled::display_menu() {
    if (menu.size() > 8) return;

    for (const auto& element : menu) {
        draw_string(0, 4 + 8 * element.id, 1, element.label);
    }
}

void Oled::i2c_write(const uint8_t *src, size_t length) {
    i2c_write_blocking(i2c_instance, address, src, length, false);
}

void Oled::clear() {
    std::fill(buffer.begin(), buffer.end(), 0);
}

void Oled::draw_pixel(uint32_t x, uint32_t y, bool inverse) {
    if(x >= (width) ||
       y >= (height)) return;

    if (inverse) {
        buffer[x+(width)*(y>>3)]&=~(0x1<<(y&0x07));
    } else {
        buffer[x+(width)*(y>>3)]|=0x1<<(y&0x07);
    }
}

void Oled::draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    if(x1>x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    if(x1==x2) {
        if(y1>y2) {
            std::swap(y1, y2);
        }

        for(int32_t i=y1; i<=y2; ++i) {
            draw_pixel(x1, i);
        }

        return;
    }

    float m = static_cast<float>(y2-y1) / static_cast<float>(x2-x1);
    for(int32_t i=x1; i<=x2; ++i) {
        float y = m * static_cast<float>(i-x1) + static_cast<float>(y1);
        draw_pixel(i, static_cast<uint32_t>(y));
    }
}

void Oled::draw_square(uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool inverse) {
    for(uint32_t i=0; i<width; ++i) {
        for(uint32_t j=0; j<height; ++j) {
            draw_pixel(x+i, y+j, inverse);
        }
    }
}

void Oled::draw_char(uint32_t x, uint32_t y, uint32_t scale, char c, bool inverse) {
    if(c<font[3]||c>font[4])
        return;

    uint32_t parts_per_line=(font[0]>>3)+((font[0]&7)>0);

    for(uint8_t w=0; w<font[1]; ++w) { // width
        uint32_t pp=(c-font[3])*font[1]*parts_per_line+w*parts_per_line+5;
        for(uint32_t lp=0; lp<parts_per_line; ++lp) {
            uint8_t line=font[pp];

            for(int8_t j=0; j<8; ++j, line>>=1) {
                if(line & 1) {
                    draw_square(x+w*scale, y+((lp<<3)+j)*scale, scale, scale, inverse);
                }
            }

            ++pp;
        }
    }
}

void Oled::draw_string(uint32_t x, uint32_t y, uint32_t scale, const std::string& s, bool inverse) {
    int32_t current_x = x;
    for (char c : s) {
        draw_char(current_x, y, scale, c, inverse);
        current_x += (font[1] + font[2]) * scale;
    }
}
void Oled::show() {
    std::vector<uint8_t> cmds = {
        SET_COL_ADDR, 0, width - 1,
        SET_PAGE_ADDR, 0, pages - 1
    };

    if(width==64) {
        cmds[1]+=32;
        cmds[2]+=32;
    }

    for(const auto& cmd : cmds) {
        uint8_t src[2]= {0x00, cmd};
        i2c_write(src, 2);
    }

    std::vector<uint8_t> transmit_buffer;
    transmit_buffer.reserve(buffer.size() + 1);
    transmit_buffer.push_back(0x40);
    transmit_buffer.insert(transmit_buffer.end(), buffer.begin(), buffer.end());
    i2c_write(transmit_buffer.data(), transmit_buffer.size());
}