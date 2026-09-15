/*
GMT130-V1.0
IPS 240*240
*/

#ifndef TFT_H
#define TFT_H

#include <stdint.h>

#define TFT_BLACK  0x0000
#define TFT_RED    0xF800
#define TFT_GREEN  0x07E0
#define TFT_BLUE   0x001F
#define TFT_WHITE  0xFFFF

void TFT_Init(void);
void TFT_FillScreen(uint16_t color);
void TFT_FillRect(uint16_t x, uint16_t y,
                  uint16_t width, uint16_t height,
                  uint16_t color);

void TFT_DrawChar(uint16_t x, uint16_t y, char ch,
                  uint16_t color, uint16_t background,
                  uint8_t scale);

void TFT_DrawString(uint16_t x, uint16_t y, const char *text,
                    uint16_t color, uint16_t background,
                    uint8_t scale);

#endif