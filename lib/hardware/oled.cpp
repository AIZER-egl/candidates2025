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
    buffer = static_cast<uint8_t*>(malloc(bufsize + 1));
    if(buffer == NULL) {
        bufsize=0;
        return false;
    }

    ++(buffer);

    std::vector<uint8_t> cmds = {
        SET_DISP,
        SET_DISP_CLK_DIV,
        0x80,
        SET_MUX_RATIO,
        static_cast<uint8_t>(height - 1),
        SET_DISP_OFFSET,
        0x00,
        SET_DISP_START_LINE,
        SET_CHARGE_PUMP,
        static_cast<uint8_t>(external_vcc?0x10:0x14),
        SET_SEG_REMAP | 0x01,           // column addr 127 mapped to SEG0
        SET_COM_OUT_DIR | 0x08,         // scan from COM[N] to COM0
        SET_COM_PIN_CFG,
        static_cast<uint8_t>(width>2*height?0x02:0x12),
        SET_CONTRAST,
        0xff,
        SET_PRECHARGE,
        static_cast<uint8_t>(external_vcc?0x22:0xF1),
        SET_VCOM_DESEL,
        0x30,                           // or 0x40?
        SET_ENTIRE_ON,                  // output follows RAM contents
        SET_NORM_INV,
        SET_DISP | 0x01,
        SET_MEM_ADDR,
        0x00,
    };


    for(const auto& cmd : cmds) {
        uint8_t src[2]= {0x00, cmd};
        write(src, 2);
    }

    return true;
}

void Oled::write(const uint8_t *src, size_t length) {
    i2c_write_blocking(i2c_instance, address, src, length, false);
}

void Oled::clear() {
    memset(buffer, 0, bufsize);
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

void Oled::draw_string(uint32_t x, uint32_t y, uint32_t scale, const char *s, bool inverse) {
    for(int32_t x_n=x; *s; x_n+=(font[1]+font[2])*scale) {
        draw_char(x_n, y, scale, *(s++), inverse);
    }
}

void Oled::show() {
    std::vector<uint8_t> cmds = {
        SET_COL_ADDR,
        0,
        static_cast<uint8_t>(width-1),
        SET_PAGE_ADDR,
        0,
        static_cast<uint8_t>(pages-1)
    };

    if(width==64) {
        cmds[1]+=32;
        cmds[2]+=32;
    }

    for(const auto& cmd : cmds) {
        uint8_t src[2]= {0x00, cmd};
        write(src, 2);
    }

    *(buffer-1)=0x40;

    write(buffer-1, bufsize+1);
}

