#ifndef BOARD_OLED_H
#define BOARD_OLED_H
#include "stm32f4xx_hal.h"
#define BOARD_OLED_ADDRESS 0x3CU /* 7-bit address; change to 0x3D if needed. */
HAL_StatusTypeDef Board_OLED_Init(void);
HAL_StatusTypeDef Board_OLED_Write(uint8_t control, const uint8_t *data, uint16_t size);
#endif
