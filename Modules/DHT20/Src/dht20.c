#include "dht20.h"
#include "i2c.h"

/* STM32 HAL nhận địa chỉ 7-bit dịch trái một bit. */
#define DHT20_ADDRESS  (0x38U << 1)
#define I2C_TIMEOUT    100U

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

DHT20_Status DHT20_Init(void)
{
    uint8_t status;

    HAL_Delay(100);

    if (HAL_I2C_IsDeviceReady(&hi2c1, DHT20_ADDRESS,
                              3, I2C_TIMEOUT) != HAL_OK)
    {
        return DHT20_ERROR_I2C;
    }

    /*
     * Đọc trực tiếp status:
     * HAL tự tạo byte địa chỉ đọc 0x71 từ địa chỉ 0x38.
     */
    if (HAL_I2C_Master_Receive(&hi2c1, DHT20_ADDRESS,
                               &status, 1, I2C_TIMEOUT) != HAL_OK)
    {
        return DHT20_ERROR_I2C;
    }

    if ((status & 0x80U) != 0U ||
        (status & 0x18U) != 0x18U)
    {
        return DHT20_ERROR_NOT_READY;
    }

    HAL_Delay(10);
    return DHT20_OK;
}

DHT20_Status DHT20_Read(DHT20_Data *data)
{
    uint8_t command[] = {0xAC, 0x33, 0x00};
    uint8_t buffer[7];

    if (data == 0)
        return DHT20_ERROR_ARGUMENT;

    if (HAL_I2C_Master_Transmit(&hi2c1, DHT20_ADDRESS,
                                command, sizeof(command),
                                I2C_TIMEOUT) != HAL_OK)
    {
        return DHT20_ERROR_I2C;
    }

    HAL_Delay(85);

    /* Đọc lại có giới hạn nếu cảm biến vẫn đang đo. */
    for (uint8_t attempt = 0; attempt < 10U; attempt++)
    {
        if (HAL_I2C_Master_Receive(&hi2c1, DHT20_ADDRESS,
                                   buffer, sizeof(buffer),
                                   I2C_TIMEOUT) != HAL_OK)
        {
            return DHT20_ERROR_I2C;
        }

        if ((buffer[0] & 0x80U) == 0U)
        {
            if (DHT20_CRC8(buffer, 6) != buffer[6])
                return DHT20_ERROR_CRC;

            if ((buffer[0] & 0x18U) != 0x18U)
                return DHT20_ERROR_NOT_READY;

            uint32_t raw_humidity =
                ((uint32_t)buffer[1] << 12) |
                ((uint32_t)buffer[2] << 4) |
                ((uint32_t)buffer[3] >> 4);

            uint32_t raw_temperature =
                ((uint32_t)(buffer[3] & 0x0FU) << 16) |
                ((uint32_t)buffer[4] << 8) |
                (uint32_t)buffer[5];

            data->humidity =
                (float)raw_humidity * 100.0f / 1048576.0f;

            data->temperature =
                (float)raw_temperature * 200.0f / 1048576.0f
                - 50.0f;

            return DHT20_OK;
        }

        HAL_Delay(10);
    }

    return DHT20_ERROR_TIMEOUT;
}