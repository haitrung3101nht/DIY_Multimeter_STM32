#include "st7789.h"

static ST7789_Write transport;
static bool ready;

static bool command(uint8_t cmd, const uint8_t *data, uint16_t size)
{
    return transport(false, &cmd, 1) && (!size || transport(true, data, size));
}

bool ST7789_Init(ST7789_Write write, ST7789_Delay delay)
{
    ready = false;
    transport = write;
    if (!write || !delay) return false;
    if (!command(0x01, 0, 0)) return false; /* Software reset */
    delay(150);
    if (!command(0x11, 0, 0)) return false; /* Sleep out */
    delay(120);
    const uint8_t format = 0x55; /* RGB565 */
    const uint8_t orientation = ST7789_MADCTL;
    if (!command(0x3A, &format, 1) || !command(0x36, &orientation, 1) ||
        !command(ST7789_INVERT ? 0x21 : 0x20, 0, 0) ||
        !command(0x13, 0, 0)) return false;
    delay(10);
    ready = true;
    /* Clear controller RAM before enabling visible output. */
    if (!ST7789_Fill(ST7789_BLACK) || !command(0x29, 0, 0)) {
        ready = false;
        return false;
    }
    delay(20);
    return true;
}

static bool window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint16_t x0 = x + ST7789_X_OFFSET, y0 = y + ST7789_Y_OFFSET;
    uint16_t x1 = x0 + w - 1U, y1 = y0 + h - 1U;
    const uint8_t columns[] = {x0 >> 8, x0 & 255, x1 >> 8, x1 & 255};
    const uint8_t rows[] = {y0 >> 8, y0 & 255, y1 >> 8, y1 & 255};
    return command(0x2A, columns, 4) && command(0x2B, rows, 4) &&
           command(0x2C, 0, 0);
}

bool ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (!ready) return false;
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT || !w || !h) return true;
    if (w > ST7789_WIDTH - x) w = ST7789_WIDTH - x;
    if (h > ST7789_HEIGHT - y) h = ST7789_HEIGHT - y;
    if (!window(x, y, w, h)) return false;
    /* Small chunks avoid a 115200-byte framebuffer and HAL's uint16 size limit. */
    uint8_t pixels[128];
    for (unsigned i = 0; i < sizeof(pixels); i += 2) {
        pixels[i] = color >> 8;
        pixels[i + 1] = color & 255;
    }
    uint32_t remaining = (uint32_t)w * h;
    while (remaining) {
        uint16_t count = remaining > 64U ? 64U : (uint16_t)remaining;
        if (!transport(true, pixels, count * 2U)) return false;
        remaining -= count;
    }
    return true;
}

bool ST7789_Fill(uint16_t color)
{
    return ST7789_FillRect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

/* Five bits per row, most significant visible bit at the left. */
static const uint8_t letters[][7] = {
    {14,17,17,31,17,17,17}, /* A */
    {30,17,17,30,17,17,30}, {14,17,16,16,16,17,14},
    {30,17,17,17,17,17,30}, {31,16,16,30,16,16,31},
    {31,16,16,30,16,16,16}, {14,17,16,23,17,17,15},
    {17,17,17,31,17,17,17}, {14,4,4,4,4,4,14},
    {7,2,2,2,18,18,12}, {17,18,20,24,20,18,17},
    {16,16,16,16,16,16,31}, {17,27,21,21,17,17,17},
    {17,25,21,19,17,17,17}, {14,17,17,17,17,17,14},
    {30,17,17,30,16,16,16}, {14,17,17,17,21,18,13},
    {30,17,17,30,20,18,17}, {15,16,16,14,1,1,30},
    {31,4,4,4,4,4,4}, {17,17,17,17,17,17,14},
    {17,17,17,17,17,10,4}, {17,17,17,21,21,21,10},
    {17,17,10,4,10,17,17}, {17,17,10,4,4,4,4},
    {31,1,2,4,8,16,31} /* Z */
};
static const uint8_t digits[][7] = {
    {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14},
    {14,17,1,2,4,8,31}, {30,1,1,14,1,1,30},
    {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
    {14,16,16,30,17,17,14}, {31,1,2,4,8,8,8},
    {14,17,17,14,17,17,14}, {14,17,17,15,1,1,14}
};

static uint8_t glyph_row(unsigned char c, unsigned row)
{
    static const uint8_t unknown[] = {14,17,1,2,4,0,4};
    if (row >= 7) return 0;
    if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
    if (c >= 'A' && c <= 'Z') return letters[c - 'A'][row];
    if (c >= '0' && c <= '9') return digits[c - '0'][row];
    switch (c) {
    case ' ': return 0;
    case ':': return (row == 2 || row == 5) ? 4 : 0;
    case '-': return row == 3 ? 14 : 0;
    case '.': return row == 6 ? 4 : 0;
    case '!': return (row < 5 || row == 6) ? 4 : 0;
    default: return unknown[row];
    }
}

bool ST7789_Text(uint16_t x, uint16_t y, const char *text,
                 uint16_t foreground, uint16_t background, uint8_t scale)
{
    if (!ready || !text || scale < 1 || scale > 4) return false;
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return true;
    uint32_t cursor_x = x, cursor_y = y;
    for (; *text; ++text) {
        if (*text == '\n') {
            cursor_x = x;
            cursor_y += 8U * scale;
            if (cursor_y >= ST7789_HEIGHT) break;
            continue;
        }
        if (cursor_x >= ST7789_WIDTH) continue;
        uint16_t w = 6U * scale, h = 8U * scale;
        if (w > ST7789_WIDTH - cursor_x) w = ST7789_WIDTH - cursor_x;
        if (h > ST7789_HEIGHT - cursor_y) h = ST7789_HEIGHT - cursor_y;
        if (!window(cursor_x, cursor_y, w, h)) return false;
        uint8_t pixels[48]; /* One scaled glyph row; no full-screen buffer. */
        for (unsigned row = 0; row < h; ++row) {
            uint8_t bits = glyph_row((unsigned char)*text, row / scale);
            for (unsigned col = 0; col < w; ++col) {
                unsigned bit = col / scale;
                uint16_t color = (bit < 5 && (bits & (16U >> bit)))
                    ? foreground : background;
                pixels[col * 2] = color >> 8;
                pixels[col * 2 + 1] = color & 255;
            }
            if (!transport(true, pixels, w * 2U)) return false;
        }
        cursor_x += 6U * scale;
    }
    return true;
}
