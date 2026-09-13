#include "board_dht20.h"
#include "dht20.h"
#include "i2c.h"

bool Board_DHT20_Write(const uint8_t *data, uint16_t size)
{
    if (!data || !size) return false;
    return HAL_I2C_Master_Transmit(&hi2c1, DHT20_I2C_ADDRESS << 1,
                                  (uint8_t *)data, size, 25) == HAL_OK;
}

bool Board_DHT20_Read(uint8_t *data, uint16_t size)
{
    if (!data || !size) return false;
    return HAL_I2C_Master_Receive(&hi2c1, DHT20_I2C_ADDRESS << 1,
                                 data, size, 25) == HAL_OK;
}

void Board_DHT20_Delay(uint32_t ms) { HAL_Delay(ms); }
