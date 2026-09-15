#ifndef DHT20_H
#define DHT20_H

#include "stm32f4xx_hal.h"

typedef struct
{
    float temperature; /* Celsius */
    float humidity;    /* %RH */
} DHT20_Data;

typedef enum
{
    DHT20_OK = 0,
    DHT20_ERROR_I2C,
    DHT20_ERROR_NOT_READY,
    DHT20_ERROR_TIMEOUT,
    DHT20_ERROR_CRC,
    DHT20_ERROR_ARGUMENT,
    DHT20_BUSY
} DHT20_Status;

DHT20_Status DHT20_Init(void);
DHT20_Status DHT20_CheckReady(void);
DHT20_Status DHT20_StartMeasurement(void);
DHT20_Status DHT20_ReadResult(DHT20_Data *data);

#endif
