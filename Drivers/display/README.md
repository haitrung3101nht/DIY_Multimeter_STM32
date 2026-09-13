# GMT130-V1.0 TFT with DHT20; optional SSD1306 OLED

## SSD1306 128x32 over I2C1

SSD1306 is temporarily disabled by default: `APP_ENABLE_OLED` is `0` in
`Core/Inc/app.h`. Set it to `1` and rebuild to run both screens together.
When disabled, the app skips all OLED initialization, updates and retries;
the OLED driver sources and CubeMX I2C1 configuration remain available.

When enabled, the OLED shows four lines: centered "XIN CHAO!", uptime in
seconds, PA0 button state, and the display types.

| OLED pin | STM32F411 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| SCL | PB6 (I2C1_SCL) |
| SDA | PB7 (I2C1_SDA) |

The existing CubeMX I2C1 configuration is 100 kHz. The 7-bit address is 0x3C
in Inc/board_oled.h (change to 0x3D only for a module configured that way).
Use a module with SDA/SCL pull-ups to 3V3, or add external pull-ups if absent.
The user confirmed the existing 128x32 panel configuration.

When enabled, the OLED uses a 512-byte framebuffer and shares the 5x7 font with the TFT.
Each screen has independent init/update status and retries failures after
one second. Updates are blocking: an OLED frame takes about 47 ms; HAL I2C
operations have a 100 ms timeout. OLED redraws only when seconds/button state
changes, with at least 100 ms between attempts to limit traffic during bounce.
An absent OLED does not prevent the TFT from updating, though an I2C timeout
can briefly delay the main loop. `oled_ready` reports successful OLED writes.

## GMT130-V1.0 / ST7789 240x240

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

The TFT now shows DHT20 temperature, humidity, sensor status and PA0 state.
See ../sensors/README.md for sensor wiring and measurement behavior.
Vietnamese labels use unaccented uppercase letters in the built-in 5x7 font.
Only the data and status fields redraw when a sample/error or button state changes.
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
cc -std=c11 -Wall -Wextra -Werror -IDrivers/display/Inc tests/test_st7789.c Drivers/display/Src/st7789.c Drivers/display/Src/font5x7.c -o /tmp/test_st7789
/tmp/test_st7789
cc -std=c11 -Wall -Wextra -Werror -IDrivers/display/Inc tests/test_ssd1306.c Drivers/display/Src/ssd1306.c Drivers/display/Src/font5x7.c -o /tmp/test_ssd1306
/tmp/test_ssd1306
```

Hardware acceptance: check legible text, sensor readings updating,
button status changing on PA0 and the LED heartbeat. Build/host tests do not establish
that the physical display works. With `APP_ENABLE_OLED=1`, check both displays
and confirm that TFT updates continue when the OLED is absent.
Disconnect wiring with power off.
