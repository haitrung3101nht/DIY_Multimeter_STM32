# DHT20 on STM32F411 / GMT130-V1.0

DHT20 measures temperature and relative humidity over the existing CubeMX
I2C1 bus at 100 kHz. It uses 7-bit address 0x38; the HAL transport shifts this
address once. The CS-less GMT130 remains on SPI1 mode 3 (PA5/PA7, PB0/PB1).
SSD1306 is temporarily disabled with APP_ENABLE_OLED=0 in Core/Inc/app.h.

## Wiring

Disconnect power before wiring. Follow the pin labels on a breakout module;
the numbered pins below refer to the DHT20 sensor datasheet.

| DHT20 | STM32F411 |
| --- | --- |
| Pin 1, VDD | 3V3 |
| Pin 2, SDA | PB7 |
| Pin 3, GND | GND |
| Pin 4, SCL | PB6 |

Share 3V3/GND with the TFT using breadboard power rails. Fit one 4.7 kOhm
pull-up from SDA to 3V3 and one from SCL to 3V3 if the module/bus does not
already provide them. For a bare sensor, add 100 nF between VDD and GND
close to the sensor. STM32 I2C pins use open-drain outputs.

## Operation

The display shows temperature in C and humidity in %, with one decimal
place. This is display precision, not a claim about sensor accuracy.
Measurements start at least two seconds apart. The main loop waits at least
85 ms before reading, polls busy responses at least 10 ms apart and aborts
a conversion after 250 ms. The wait between transfers does not block TFT,
LED or button processing. Individual HAL I2C operations use a 25 ms timeout;
sensor initialization may delay for power stabilization/register recovery.

Each seven-byte sample must pass the status and CRC8 checks before it is
published. On failure, old readings are replaced by placeholders and
"LOI DOC CAM BIEN". Initialization/measurement is retried every two seconds.
`dht20_valid` in the debugger is 1 only after a successful sample; reading
this flag does not prove the actual pixels are visible on the TFT.

## Build and tests

From the project root:

```sh
cmake --preset Debug
cmake --build --preset Debug
cc -std=c11 -Wall -Wextra -Werror -IDrivers/sensors/Inc tests/test_dht20.c Drivers/sensors/Src/dht20.c -o /tmp/test_dht20
/tmp/test_dht20
```

Hardware check: connect the sensor, confirm plausible readings update, then
power off and disconnect it; on the next boot, placeholders/error should
appear while the TFT and PC13 LED continue running. Host tests and Flash
verification do not replace this visual/electrical check.

Datasheet (Aosong, pin definitions, pull-ups, timing, CRC and conversion):
https://www.mouser.com/datasheet/2/813/DHT20-2498062.pdf
