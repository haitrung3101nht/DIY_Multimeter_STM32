# GMT130-V1.0 / ST7789 240x240

Target: GMT130-V1.0, 1.3-inch IPS ST7789 240x240, seven pins with no CS.
CubeMX owns GPIO/SPI1 initialization. Call app_init after MX_GPIO_Init and
MX_SPI1_Init. Keep the generated TFT_RST and TFT_DC pin labels.

Wiring: SCK -> PA5, SDA -> PA7, RES -> PB0, DC -> PB1,
VCC -> 3V3, GND -> GND, BLK (backlight control) -> 3V3 for always-on light.
PA4 is unused. SPI1 must be dedicated to this module because it has no CS.

SPI1 uses mode 3 (CPOL HIGH, CPHA 2EDGE), 8-bit MSB first at 8 MHz.
Both spi.c and STMproject.ioc carry these settings. The board transport enables
SPI while reset is LOW, so SCK is already idle HIGH when reset is released.
This follows TFT_eSPI's mode-3 compatibility default for ST7789 modules;
actual screen output still needs visual confirmation.
Module identification/pin reference:
https://goldenmorninglcd.com/tft-display-module/1.3-inch-240x240-st7789-gmt130-v1.0/
Mode reference:
https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_eSPI.h

Demo: "XIN CHAO!", "STM32F411 TFT 240X240", elapsed seconds since reset,
and PA0 button status ("DANG NHAN" when held to GND, "DA THA" when released).
Vietnamese labels use unaccented uppercase letters in the built-in 5x7 font.
Only the time and button fields redraw when their values change.
PC13 LED still toggles every 500 ms. Startup and SPI writes are blocking.
Transport failures trigger initialization retries after one second.
`tft_ready` means HAL writes succeeded, not that the display is connected:
write-only SPI cannot detect a missing screen.

Panel defaults in Inc/st7789.h: MADCTL 0xC0, X offset 0, Y offset 80,
inversion enabled. These may need adjustment for another panel orientation.
Reference for initialization and 240x240 address mapping:
https://github.com/adafruit/Adafruit-ST7735-Library/blob/master/Adafruit_ST7789.cpp

Build and flash from project root:

```sh
cmake --preset Debug
cmake --build --preset Debug
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program build/Debug/STMproject.elf verify reset exit"
```

Host tests (no board required):

```sh
cc -std=c11 -Wall -Wextra -Werror -IDrivers/display/Inc tests/test_st7789.c Drivers/display/Src/st7789.c -o /tmp/test_st7789
/tmp/test_st7789
```

Hardware acceptance: check legible text, the seconds counter advancing,
button status changing on PA0 and the LED heartbeat. Build/host tests do not establish
that the physical display works.
