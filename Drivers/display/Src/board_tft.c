#include "board_tft.h"
#include "main.h"
#include "spi.h"

bool Board_TFT_Init(void)
{
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET);
    if (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY) return false;
    HAL_Delay(10);
    HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET);
    /* GMT130-V1.0 has no CS. Settle mode-3 SCK HIGH while reset is asserted,
       before the panel can interpret the first rising edge as a data bit. */
    __HAL_SPI_ENABLE(&hspi1);
    HAL_Delay(20);
    HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(120);
    return true;
}

bool Board_TFT_Write(bool data, const uint8_t *bytes, uint16_t size)
{
    if (!bytes || !size) return false;
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin,
                      data ? GPIO_PIN_SET : GPIO_PIN_RESET);
    /* The CS-less module uses SPI1 exclusively; PA4 is not connected. */
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, (uint8_t *)bytes, size, 100);
    return status == HAL_OK;
}

void Board_TFT_Delay(uint32_t ms) { HAL_Delay(ms); }
