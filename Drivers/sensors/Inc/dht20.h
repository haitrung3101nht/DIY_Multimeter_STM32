#ifndef DHT20_H
#define DHT20_H

#include <stdbool.h>
#include <stdint.h>

#define DHT20_I2C_ADDRESS 0x38U /* 7-bit address; shift left for STM32 HAL. */

/* Callbacks transfer exactly size bytes, returning false on any bus error. */
typedef bool (*DHT20_Write)(const uint8_t *data, uint16_t size);
typedef bool (*DHT20_Read)(uint8_t *data, uint16_t size);
typedef void (*DHT20_Delay)(uint32_t milliseconds);

typedef struct {
    int16_t temperature_tenths; /* Degrees Celsius, rounded to 0.1 C. */
    uint16_t humidity_tenths;   /* Relative humidity, rounded to 0.1 %. */
} DHT20_Sample;

typedef enum DHT20_Status {
    DHT20_OK,
    DHT20_BUSY,
    DHT20_ERROR
} DHT20_Status;

/* Single sensor. Blocks for power stabilization and calibration recovery.
 * On failure, Start/ReadSample remain disabled until Init succeeds. */
bool DHT20_Init(DHT20_Write write, DHT20_Read read, DHT20_Delay delay);

/* Sends the measurement command without waiting for conversion. The caller
 * should allow 2 seconds between starts, then wait at least 80 ms before ReadSample.
 * A new Start discards any previous pending conversion. */
bool DHT20_Start(void);

/* Read a pending measurement. On BUSY, wait and retry with a bounded timeout.
 * On OK/error, call Start for the next measurement. Output is unchanged unless
 * OK is returned; only calibrated samples with a valid CRC are accepted. */
DHT20_Status DHT20_ReadSample(DHT20_Sample *sample);

#endif
