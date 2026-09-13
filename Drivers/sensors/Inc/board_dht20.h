#ifndef BOARD_DHT20_H
#define BOARD_DHT20_H
#include <stdbool.h>
#include <stdint.h>
/* Uses CubeMX I2C1 on PB6/PB7; call after MX_I2C1_Init(). */
bool Board_DHT20_Write(const uint8_t *data, uint16_t size);
bool Board_DHT20_Read(uint8_t *data, uint16_t size);
void Board_DHT20_Delay(uint32_t ms);
#endif
