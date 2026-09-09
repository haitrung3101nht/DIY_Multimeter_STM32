#ifndef BOARD_TFT_H
#define BOARD_TFT_H
#include <stdbool.h>
#include <stdint.h>
/* Call after CubeMX GPIO and SPI1 initialization. */
bool Board_TFT_Init(void);
bool Board_TFT_Write(bool data, const uint8_t *bytes, uint16_t size);
void Board_TFT_Delay(uint32_t ms);
#endif
