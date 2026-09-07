/* Layout/font only: eight 5x7 glyphs scaled 2x, centered on 128x32. */
#include "clock_view.h"
#include "ssd1306.h"
static const uint8_t digits[10][5] = {
    {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E}
};
bool ClockView_Show(const char text[12])
{
    static const uint8_t colon[5] = {0,0x36,0x36,0,0};
    SSD1306_Clear();
    for (unsigned i = 0; i < 8; ++i) {
        const uint8_t *glyph;
        if (text[i] == ':') glyph = colon;
        else if (text[i] >= '0' && text[i] <= '9') glyph = digits[text[i] - '0'];
        else continue;
        for (unsigned x = 0; x < 5; ++x)
            for (unsigned y = 0; y < 7; ++y)
                if (glyph[x] & (1U << y))
                    for (unsigned dx = 0; dx < 2; ++dx)
                        for (unsigned dy = 0; dy < 2; ++dy)
                            SSD1306_Pixel(17 + i * 12 + x * 2 + dx, 9 + y * 2 + dy);
    }
    
    return SSD1306_Flush();
}
