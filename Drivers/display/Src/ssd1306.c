/* SSD1306 128x32 framebuffer and controller commands, independent of HAL. */
#include "ssd1306.h"
#include <string.h>
static uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8U];
static SSD1306_Write transport;
bool SSD1306_Init(SSD1306_Write write)
{
    static const uint8_t setup[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x1F, 0xD3, 0x00, 0x40,
        0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x02,
        0x81, 0x8F, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6
    };
    const uint8_t on = 0xAF;
    transport = write;
    if (!transport || !transport(0x00, setup, sizeof(setup))) return false;
    SSD1306_Clear();
    return SSD1306_Flush() && transport(0x00, &on, 1);
}
void SSD1306_Clear(void) { memset(buffer, 0, sizeof(buffer)); }
void SSD1306_Pixel(uint16_t x, uint16_t y)
{
    if (x < SSD1306_WIDTH && y < SSD1306_HEIGHT)
        buffer[x + (y / 8U) * SSD1306_WIDTH] |= (uint8_t)(1U << (y % 8U));
}
bool SSD1306_Flush(void)
{
    const uint8_t window[] = {0x21, 0, SSD1306_WIDTH - 1, 0x22, 0, SSD1306_HEIGHT / 8 - 1};
    return transport && transport(0x00, window, sizeof(window)) &&
           transport(0x40, buffer, sizeof(buffer));
}
