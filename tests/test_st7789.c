#include "st7789.h"
#include <assert.h>
#include <stdio.h>

static uint8_t cmd, columns[4], rows[4];
static unsigned pixel_bytes, calls, windows, cursor, fail_at_call;
static uint16_t screen[ST7789_HEIGHT][ST7789_WIDTH];
static bool fail, check_color;
static bool write_mock(bool data, const uint8_t *p, uint16_t n)
{
    ++calls;
    if (fail || (fail_at_call && calls == fail_at_call)) return false;
    assert(p && n);
    if (!data) {
        assert(n == 1);
        cmd = *p;
        if (cmd == 0x2C) {
            cursor = 0;
            ++windows;
        }
    }
    else if (cmd == 0x2A || cmd == 0x2B) {
        assert(n == 4);
        for (unsigned i = 0; i < 4; ++i)
            (cmd == 0x2A ? columns : rows)[i] = p[i];
    } else if (cmd == 0x2C) {
        assert(n % 2 == 0 && n <= 128);
        pixel_bytes += n;
        unsigned x0 = ((unsigned)columns[0] << 8) | columns[1];
        unsigned y0 = ((unsigned)rows[0] << 8) | rows[1];
        unsigned x1 = ((unsigned)columns[2] << 8) | columns[3];
        unsigned y1 = ((unsigned)rows[2] << 8) | rows[3];
        assert(x1 >= x0);
        assert(y0 >= ST7789_Y_OFFSET && y1 >= y0);
        unsigned width = x1 - x0 + 1;
        unsigned height = y1 - y0 + 1;
        for (unsigned i = 0; i < n; i += 2) {
            assert(cursor < width * height);
            unsigned x = x0 - ST7789_X_OFFSET + cursor % width;
            unsigned y = y0 - ST7789_Y_OFFSET + cursor / width;
            assert(x < ST7789_WIDTH && y < ST7789_HEIGHT);
            screen[y][x] = ((uint16_t)p[i] << 8) | p[i + 1];
            if (check_color) assert(screen[y][x] == ST7789_RED);
            ++cursor;
        }
    }
    return true;
}
static void delay_mock(uint32_t ms) { assert(ms > 0); }

static void assert_letter_a(unsigned x, unsigned y, unsigned scale)
{
    static const uint8_t expected_rows[] = {
        0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11
    };
    for (unsigned row = 0; row < 8 * scale; ++row) {
        for (unsigned col = 0; col < 6 * scale; ++col) {
            bool ink = row / scale < 7 && col / scale < 5 &&
                (expected_rows[row / scale] & (1U << (4 - col / scale)));
            assert(screen[y + row][x + col] ==
                   (ink ? ST7789_WHITE : ST7789_BLACK));
        }
    }
}

static void test_text(void)
{
    check_color = false;
    assert(ST7789_Fill(ST7789_GREEN));
    unsigned before_windows = windows;
    pixel_bytes = 0;
    assert(ST7789_Text(10, 20, "A ", ST7789_WHITE, ST7789_BLACK, 1));
    assert(windows == before_windows + 2);
    assert(pixel_bytes == 2 * 6 * 8 * 2);
    assert_letter_a(10, 20, 1);
    for (unsigned y = 20; y < 28; ++y)
        for (unsigned x = 16; x < 22; ++x)
            assert(screen[y][x] == ST7789_BLACK);
    assert(screen[19][10] == ST7789_GREEN);
    assert(screen[20][9] == ST7789_GREEN);
    assert(screen[20][22] == ST7789_GREEN);
    assert(screen[28][10] == ST7789_GREEN);

    assert(ST7789_Text(30, 40, "a", ST7789_WHITE, ST7789_BLACK, 2));
    assert_letter_a(30, 40, 2);
    assert(ST7789_Text(120, 120, "A", ST7789_WHITE, ST7789_BLACK, 4));
    assert_letter_a(120, 120, 4);
    assert(ST7789_Text(60, 70, "A\nA", ST7789_WHITE, ST7789_BLACK, 1));
    assert_letter_a(60, 70, 1);
    assert_letter_a(60, 78, 1);
    assert(screen[70][66] == ST7789_GREEN);

    assert(ST7789_Text(90, 90, "?@", ST7789_WHITE, ST7789_BLACK, 1));
    for (unsigned row = 0; row < 8; ++row)
        for (unsigned col = 0; col < 6; ++col)
            assert(screen[90 + row][90 + col] ==
                   screen[90 + row][96 + col]);

    pixel_bytes = 0;
    assert(ST7789_Text(239, 239, "AA", ST7789_WHITE, ST7789_BLACK, 1));
    assert(pixel_bytes == 2);
    assert(screen[239][239] == ST7789_BLACK);
    pixel_bytes = 0;
    assert(ST7789_Text(234, 220, "AB", ST7789_WHITE, ST7789_BLACK, 1));
    assert(pixel_bytes == 6 * 8 * 2);
    assert_letter_a(234, 220, 1);
    assert(screen[228][234] == ST7789_GREEN);

    unsigned before = calls;
    assert(!ST7789_Text(0, 0, NULL, ST7789_WHITE, ST7789_BLACK, 1));
    assert(!ST7789_Text(0, 0, "A", ST7789_WHITE, ST7789_BLACK, 0));
    assert(!ST7789_Text(0, 0, "A", ST7789_WHITE, ST7789_BLACK, 5));
    assert(ST7789_Text(0, 0, "", ST7789_WHITE, ST7789_BLACK, 1));
    assert(ST7789_Text(65535, 0, "A", ST7789_WHITE, ST7789_BLACK, 1));
    assert(ST7789_Text(0, 65535, "A", ST7789_WHITE, ST7789_BLACK, 1));
    assert(calls == before);

    /* Fail after the first pixel row, while a glyph is being transmitted. */
    fail_at_call = calls + 7;
    assert(!ST7789_Text(0, 0, "AA", ST7789_WHITE, ST7789_BLACK, 1));
    assert(calls == fail_at_call);
    fail_at_call = 0;
}

int main(void)
{
    assert(!ST7789_Fill(0));
    assert(!ST7789_Text(0, 0, "A", ST7789_WHITE, ST7789_BLACK, 1));
    assert(!ST7789_Init(NULL, delay_mock));
    assert(!ST7789_Init(write_mock, NULL));
    assert(ST7789_Init(write_mock, delay_mock));
    assert(pixel_bytes == 240U * 240U * 2U);
    pixel_bytes = 0;
    check_color = true;
    assert(ST7789_FillRect(239, 239, 65535, 65535, ST7789_RED));
    assert(pixel_bytes == 2);
    assert(columns[0] == 0 && columns[1] == 239 && columns[3] == 239);
    assert(rows[0] == 1 && rows[1] == 63 && rows[2] == 1 && rows[3] == 63);
    unsigned before = calls;
    assert(ST7789_FillRect(240, 0, 10, 10, 0));
    assert(ST7789_FillRect(0, 0, 0, 10, 0));
    assert(calls == before);
    pixel_bytes = 0;
    assert(ST7789_Fill(ST7789_RED));
    assert(pixel_bytes == 115200U);
    test_text();
    fail = true;
    assert(!ST7789_Fill(ST7789_RED));
    assert(!ST7789_Init(write_mock, delay_mock));
    assert(!ST7789_Fill(0));
    puts("ST7789 tests passed");
}
