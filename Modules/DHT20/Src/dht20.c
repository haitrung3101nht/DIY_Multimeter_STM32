#include "dht20.h"
#include "i2c.h"

/* STM32 HAL nhận địa chỉ 7-bit dịch trái một bit. */
#define DHT20_ADDRESS  (0x38U << 1)
#define I2C_TIMEOUT    5U

static uint8_t DHT20_CRC8(const uint8_t *data, uint8_t length)
{
    uint8_t crc = 0xFF;

    for (uint8_t i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (uint8_t bit = 0; bit < 8U; bit++)
        {
            if (crc & 0x80U)
                crc = (uint8_t)((crc << 1) ^ 0x31U);
            else
                crc = (uint8_t)(crc << 1);
        }
    }

    return crc;
}

DHT20_Status DHT20_CheckReady(void)
{
    uint8_t status;
    if (HAL_I2C_Master_Receive(&hi2c1, DHT20_ADDRESS, &status, 1,
                               I2C_TIMEOUT) != HAL_OK)
        return DHT20_ERROR_I2C;
    if (status & 0x80U) return DHT20_BUSY;
    return (status & 0x18U) == 0x18U ? DHT20_OK : DHT20_ERROR_NOT_READY;
}

/* Startup only. Runtime retries use CheckReady(), which has no delay. */
DHT20_Status DHT20_Init(void)
{
    HAL_Delay(100);
    return DHT20_CheckReady();
}

DHT20_Status DHT20_StartMeasurement(void)
{
    uint8_t command[] = {0xAC, 0x33, 0x00};
    return HAL_I2C_Master_Transmit(&hi2c1, DHT20_ADDRESS, command,
        sizeof(command), I2C_TIMEOUT) == HAL_OK ? DHT20_OK : DHT20_ERROR_I2C;
}

/* Read once; the application schedules the 85 ms conversion wait. */
DHT20_Status DHT20_ReadResult(DHT20_Data *data)
{
    uint8_t b[7];
    if (!data) return DHT20_ERROR_ARGUMENT;
    if (HAL_I2C_Master_Receive(&hi2c1, DHT20_ADDRESS, b, sizeof(b),
                              I2C_TIMEOUT) != HAL_OK)
        return DHT20_ERROR_I2C;
    if (b[0] & 0x80U) return DHT20_BUSY;
    if (DHT20_CRC8(b, 6) != b[6]) return DHT20_ERROR_CRC;
    if ((b[0] & 0x18U) != 0x18U) return DHT20_ERROR_NOT_READY;
    uint32_t rh = ((uint32_t)b[1] << 12) | ((uint32_t)b[2] << 4) | (b[3] >> 4);
    uint32_t t = ((uint32_t)(b[3] & 15U) << 16) | ((uint32_t)b[4] << 8) | b[5];
    data->humidity = rh * (100.0f / 1048576.0f);
    data->temperature = t * (200.0f / 1048576.0f) - 50.0f;
    return DHT20_OK;
}
