#include "dht20.h"
#include <stddef.h>

static DHT20_Write sensor_write;
static DHT20_Read sensor_read;
static DHT20_Delay sensor_delay;
static bool initialized;
static bool conversion_pending;

static bool read_status(uint8_t *status)
{
    /* 0x71 in the datasheet is address 0x38 with the I2C read bit set.
     * The transport emits that address; there is no status command payload. */
    return sensor_read(status, 1);
}

/* Aosong recovery protocol for the three registers named in datasheet 7.4.
 * Manufacturer demo reference:
 * https://github.com/RobTillaart/DHT20/blob/master/DHT20.cpp (_resetRegister).
 */
static bool recover_register(uint8_t reg)
{
    uint8_t command[] = {reg, 0x00, 0x00};
    uint8_t response[3];
    if (!sensor_write(command, sizeof(command))) return false;
    sensor_delay(5);
    if (!sensor_read(response, sizeof(response))) return false;
    sensor_delay(10);
    command[0] = (uint8_t)(0xB0U | reg);
    command[1] = response[1];
    command[2] = response[2];
    if (!sensor_write(command, sizeof(command))) return false;
    sensor_delay(5);
    return true;
}

bool DHT20_Init(DHT20_Write write, DHT20_Read read, DHT20_Delay delay)
{
    initialized = false;
    conversion_pending = false;
    sensor_write = write;
    sensor_read = read;
    sensor_delay = delay;
    if (write == NULL || read == NULL || delay == NULL) return false;

    sensor_delay(100);
    uint8_t status;
    if (!read_status(&status) || (status & 0x80U)) return false;
    if ((status & 0x18U) != 0x18U) {
        if (!recover_register(0x1B) || !recover_register(0x1C) ||
            !recover_register(0x1E)) return false;
        sensor_delay(10);
        if (!read_status(&status) || (status & 0x98U) != 0x18U) return false;
    }
    sensor_delay(10);
    initialized = true;
    return true;
}

bool DHT20_Start(void)
{
    const uint8_t command[] = {0xAC, 0x33, 0x00};
    conversion_pending = initialized && sensor_write(command, sizeof(command));
    return conversion_pending;
}

static uint8_t crc8(const uint8_t *data, uint16_t size)
{
    uint8_t crc = 0xFF;
    for (uint16_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit)
            crc = (uint8_t)((crc << 1) ^ ((crc & 0x80U) ? 0x31U : 0U));
    }
    return crc;
}

DHT20_Status DHT20_ReadSample(DHT20_Sample *sample)
{
    if (!initialized || !conversion_pending || sample == NULL) return DHT20_ERROR;
    uint8_t packet[7];
    if (!sensor_read(packet, sizeof(packet))) {
        conversion_pending = false;
        return DHT20_ERROR;
    }
    if (packet[0] & 0x80U) return DHT20_BUSY;
    conversion_pending = false;
    if (!(packet[0] & 0x08U) || crc8(packet, 6) != packet[6]) return DHT20_ERROR;

    uint32_t raw_humidity = ((uint32_t)packet[1] << 12) |
                            ((uint32_t)packet[2] << 4) | (packet[3] >> 4);
    uint32_t raw_temperature = ((uint32_t)(packet[3] & 0x0FU) << 16) |
                               ((uint32_t)packet[4] << 8) | packet[5];
    /* Datasheet section 8, in tenths; products fit in uint32_t. */
    sample->humidity_tenths = (uint16_t)((raw_humidity * 1000U + 524288U) >> 20);
    sample->temperature_tenths =
        (int16_t)((int32_t)((raw_temperature * 2000U + 524288U) >> 20) - 500);
    return DHT20_OK;
}
