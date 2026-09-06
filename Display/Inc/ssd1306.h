#ifndef SSD1306_H
#define SSD1306_H
#include <stdbool.h>
#include <stdint.h>
#define SSD1306_WIDTH 128U
#define SSD1306_HEIGHT 32U
typedef bool (*SSD1306_Write)(uint8_t control, const uint8_t *data, uint16_t size);
bool SSD1306_Init(SSD1306_Write write);
void SSD1306_Clear(void);
void SSD1306_Pixel(uint16_t x, uint16_t y);
bool SSD1306_Flush(void);
#endif
