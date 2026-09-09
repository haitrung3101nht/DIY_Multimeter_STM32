#ifndef ST7789_H
#define ST7789_H
#include <stdbool.h>
#include <stdint.h>
#define ST7789_WIDTH 240U
#define ST7789_HEIGHT 240U
/* Portrait, MX/MY mirrored: visible rows 80..319 on common 240x240 panels. */
#ifndef ST7789_X_OFFSET
#define ST7789_X_OFFSET 0U
#endif
#ifndef ST7789_Y_OFFSET
#define ST7789_Y_OFFSET 80U
#endif
#ifndef ST7789_MADCTL
#define ST7789_MADCTL 0xC0U
#endif
#ifndef ST7789_INVERT
#define ST7789_INVERT 1
#endif
#define ST7789_BLACK 0x0000U
#define ST7789_WHITE 0xFFFFU
#define ST7789_RED   0xF800U
#define ST7789_GREEN 0x07E0U
#define ST7789_BLUE  0x001FU

typedef bool (*ST7789_Write)(bool data, const uint8_t *bytes, uint16_t size);
typedef void (*ST7789_Delay)(uint32_t ms);
bool ST7789_Init(ST7789_Write write, ST7789_Delay delay);
/* RGB565, clipped to the visible screen; empty/outside rectangles are no-ops. */
bool ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
bool ST7789_Fill(uint16_t color);
/* Opaque 6x8 cells, scale 1..4. A-Z, digits and basic punctuation;
 * lowercase appears uppercase, unsupported characters become '?'.
 * Clips at screen edges; '\n' starts the next line at the original x. */
bool ST7789_Text(uint16_t x, uint16_t y, const char *text,
                 uint16_t foreground, uint16_t background, uint8_t scale);
#endif
