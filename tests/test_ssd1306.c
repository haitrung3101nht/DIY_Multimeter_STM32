#include "ssd1306.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t frame[SSD1306_WIDTH * SSD1306_HEIGHT / 8U];
static unsigned calls, fail_at_call, data_calls, data_bytes, windows;
static uint8_t last_control, last_command;

static bool write_mock(uint8_t control, const uint8_t *data, uint16_t size)
{
    ++calls;
    if (fail_at_call && calls == fail_at_call) return false;
    assert(data && size);
    last_control = control;
    if (control == 0x40) {
        assert(size == sizeof(frame));
        memcpy(frame, data, size);
        ++data_calls;
        data_bytes += size;
    } else {
        assert(control == 0x00);
        last_command = data[0];
        if (data[0] == 0x21) {
            const uint8_t expected[] = {
                0x21, 0, SSD1306_WIDTH - 1,
                0x22, 0, SSD1306_HEIGHT / 8 - 1
            };
            assert(size == sizeof(expected));
            assert(memcmp(data, expected, size) == 0);
            ++windows;
        }
    }
    return true;
}

static bool screen_pixel(unsigned x, unsigned y)
{
    assert(x < SSD1306_WIDTH && y < SSD1306_HEIGHT);
    return (frame[x + (y / 8U) * SSD1306_WIDTH] & (1U << (y % 8U))) != 0;
}

static bool letter_a_pixel(unsigned col, unsigned row)
{
    static const uint8_t expected_rows[] = {
        0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11
    };
    return row < 7 && col < 5 &&
           (expected_rows[row] & (1U << (4U - col))) != 0;
}

static void assert_letter_a(unsigned x, unsigned y)
{
    for (unsigned row = 0; row < 8; ++row)
        for (unsigned col = 0; col < 6; ++col)
            assert(screen_pixel(x + col, y + row) == letter_a_pixel(col, row));
}

static void flush_once(void)
{
    unsigned before_calls = calls;
    unsigned before_data_calls = data_calls;
    unsigned before_data_bytes = data_bytes;
    unsigned before_windows = windows;
    assert(SSD1306_Flush());
    assert(calls == before_calls + 2);
    assert(data_calls == before_data_calls + 1);
    assert(data_bytes == before_data_bytes + sizeof(frame));
    assert(windows == before_windows + 1);
}

static void fill_pixels(void)
{
    for (unsigned y = 0; y < SSD1306_HEIGHT; ++y)
        for (unsigned x = 0; x < SSD1306_WIDTH; ++x)
            SSD1306_Pixel(x, y);
}

static void test_init(void)
{
    assert(!SSD1306_Flush());
    assert(!SSD1306_Init(NULL));
    assert(calls == 0);

    /* Setup, address window, framebuffer, and display-on must all succeed. */
    for (unsigned step = 1; step <= 4; ++step) {
        unsigned before = calls;
        fail_at_call = before + step;
        assert(!SSD1306_Init(write_mock));
        assert(calls == fail_at_call);
    }
    fail_at_call = 0;
    unsigned before_data = data_bytes;
    assert(SSD1306_Init(write_mock));
    assert(data_bytes == before_data + sizeof(frame));
    assert(last_control == 0x00 && last_command == 0xAF);
    for (unsigned i = 0; i < sizeof(frame); ++i) assert(frame[i] == 0);
}

static void test_text(void)
{
    fill_pixels();
    unsigned before = calls;
    SSD1306_Text(10, 3, "A ");
    assert(calls == before); /* Drawing changes RAM; only Flush uses I2C. */
    flush_once();
    assert_letter_a(10, 3);
    for (unsigned y = 0; y < SSD1306_HEIGHT; ++y) {
        for (unsigned x = 0; x < SSD1306_WIDTH; ++x) {
            bool expected = true;
            if (y >= 3 && y < 11 && x >= 10 && x < 22)
                expected = x < 16 && letter_a_pixel(x - 10, y - 3);
            assert(screen_pixel(x, y) == expected);
        }
    }

    /* Spaces erase old glyphs without disturbing adjacent pixels/pages. */
    SSD1306_Text(10, 3, "  ");
    flush_once();
    for (unsigned y = 3; y < 11; ++y)
        for (unsigned x = 10; x < 22; ++x)
            assert(!screen_pixel(x, y));
    assert(screen_pixel(9, 3) && screen_pixel(22, 3));
    assert(screen_pixel(10, 2) && screen_pixel(10, 11));

    SSD1306_Clear();
    SSD1306_Text(4, 2, "a\nA");
    flush_once();
    assert_letter_a(4, 2);
    assert_letter_a(4, 10);
    assert(!screen_pixel(10, 2));

    /* A newline must still work after text has reached the right edge. */
    SSD1306_Clear();
    SSD1306_Text(122, 0, "AAAA\nA");
    flush_once();
    assert_letter_a(122, 0);
    assert_letter_a(122, 8);
    assert(!screen_pixel(0, 0));

    SSD1306_Clear();
    SSD1306_Text(0, 0, "?@");
    flush_once();
    for (unsigned row = 0; row < 8; ++row)
        for (unsigned col = 0; col < 6; ++col)
            assert(screen_pixel(col, row) == screen_pixel(6 + col, row));
}

static void test_clipping(void)
{
    fill_pixels();
    SSD1306_Text(126, 28, "AA\nA");
    flush_once();
    for (unsigned y = 0; y < SSD1306_HEIGHT; ++y) {
        for (unsigned x = 0; x < SSD1306_WIDTH; ++x) {
            bool expected = x < 126 || y < 28 || letter_a_pixel(x - 126, y - 28);
            assert(screen_pixel(x, y) == expected);
        }
    }

    uint8_t saved[sizeof(frame)];
    memcpy(saved, frame, sizeof(saved));
    unsigned before = calls;
    SSD1306_Text(0, 0, NULL);
    SSD1306_Text(0, 0, "");
    SSD1306_Text(SSD1306_WIDTH, 0, "A");
    SSD1306_Text(0, SSD1306_HEIGHT, "A");
    SSD1306_Text(UINT16_MAX, 0, "A");
    SSD1306_Text(0, UINT16_MAX, "A");
    SSD1306_Pixel(UINT16_MAX, UINT16_MAX);
    assert(calls == before);
    flush_once();
    assert(memcmp(saved, frame, sizeof(frame)) == 0);

    SSD1306_Clear();
    flush_once();
    for (unsigned i = 0; i < sizeof(frame); ++i) assert(frame[i] == 0);
}

static void test_flush_failure(void)
{
    /* A failed address window must not be followed by framebuffer data. */
    unsigned before_data = data_calls;
    fail_at_call = calls + 1;
    assert(!SSD1306_Flush());
    assert(calls == fail_at_call && data_calls == before_data);

    fail_at_call = calls + 2;
    assert(!SSD1306_Flush());
    assert(calls == fail_at_call && data_calls == before_data);

    fail_at_call = 0;
    flush_once();
}

int main(void)
{
    assert(SSD1306_WIDTH == 128 && SSD1306_HEIGHT == 32);
    assert(sizeof(frame) == 512);
    test_init();
    test_text();
    test_clipping();
    test_flush_failure();
    puts("SSD1306 tests passed");
}
