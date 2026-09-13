#include "app.h"
#include "led.h"
#include "main.h"
#include "board_tft.h"
#include "st7789.h"
#include "board_dht20.h"
#include "dht20.h"
#if APP_ENABLE_OLED
#include "board_oled.h"
#include "ssd1306.h"
#endif
#include <stdio.h>

#define SCREEN_BG 0x0863U
#define SCREEN_ACCENT 0x07FFU

static uint32_t last_toggle_tick;
static uint32_t last_tft_attempt;
static uint32_t displayed_sensor_revision;
static uint32_t sensor_revision;
static uint32_t last_sensor_attempt;
static uint32_t measurement_started;
static uint32_t last_sensor_poll;
static bool sensor_initialized;
static bool sensor_measuring;
static bool sensor_failed;
static DHT20_Sample sensor_sample;
volatile uint8_t dht20_valid;
static uint8_t displayed_button;
#if APP_ENABLE_OLED
static uint32_t last_oled_attempt;
static uint32_t oled_displayed_seconds;
static uint8_t oled_displayed_button;
#endif

volatile uint32_t elapsed_ms;
volatile uint8_t button_pressed;
/* Transport success only: a write-only SPI screen cannot acknowledge presence. */
volatile uint8_t tft_ready;
#if APP_ENABLE_OLED
volatile uint8_t oled_ready;

static bool oled_write(uint8_t control, const uint8_t *data, uint16_t size)
{
    /* HAL_OK is zero; the controller callback expects true on success. */
    return Board_OLED_Write(control, data, size) == HAL_OK;
}

static bool oled_status(uint32_t seconds, uint8_t pressed)
{
    char uptime[22];
    (void)snprintf(uptime, sizeof(uptime), "CHAY: %7lu S", (unsigned long)seconds);
    SSD1306_Clear();
    SSD1306_Text(37, 0, "XIN CHAO!");
    SSD1306_Text(0, 8, uptime);
    SSD1306_Text(0, 16, pressed ? "PA0: DANG NHAN" : "PA0: DA THA");
    SSD1306_Text(0, 24, "OLED 128X32 + TFT");
    if (!SSD1306_Flush()) return false;
    oled_displayed_seconds = seconds;
    oled_displayed_button = pressed;
    return true;
}

static bool oled_init(void)
{
    return Board_OLED_Init() == HAL_OK && SSD1306_Init(oled_write) &&
           oled_status(HAL_GetTick() / 1000U, button_pressed);
}
#endif

static void sensor_error(void)
{
    dht20_valid = 0;
    sensor_failed = true;
    sensor_initialized = false;
    sensor_measuring = false;
    ++sensor_revision;
}

static void sensor_process(uint32_t now)
{
    if (sensor_measuring) {
        if ((uint32_t)(now - measurement_started) >= 250U) {
            sensor_error();
            return;
        }
        if ((uint32_t)(now - measurement_started) < 85U ||
            (uint32_t)(now - last_sensor_poll) < 10U) return;
        last_sensor_poll = now;
        DHT20_Sample next;
        DHT20_Status status = DHT20_ReadSample(&next);
        if (status == DHT20_BUSY) return;
        if (status != DHT20_OK) {
            sensor_error();
            return;
        }
        sensor_sample = next;
        dht20_valid = 1;
        sensor_failed = false;
        sensor_measuring = false;
        ++sensor_revision;
    } else if ((uint32_t)(now - last_sensor_attempt) >= 2000U) {
        if (!sensor_initialized)
            sensor_initialized = DHT20_Init(Board_DHT20_Write, Board_DHT20_Read,
                                            Board_DHT20_Delay);
        if (!sensor_initialized || !DHT20_Start()) {
            sensor_error();
        } else {
            sensor_measuring = true;
            measurement_started = HAL_GetTick();
            last_sensor_poll = measurement_started;
        }
        last_sensor_attempt = HAL_GetTick();
    }
}

static bool display_status(uint8_t pressed)
{
    char temperature[20] = "0.0  C";
    char humidity[20] = "0.0  %";
    if (dht20_valid) {
        char number[16];
        int32_t t = sensor_sample.temperature_tenths;
        int32_t magnitude = t < 0 ? -t : t;
        
        (void)snprintf(number, sizeof(number), "%s%ld.%ld C",
                       t < 0 ? "-" : "", (long)(magnitude / 10), (long)(magnitude % 10));
        (void)snprintf(temperature, sizeof(temperature), "%-9s", number);
        (void)snprintf(number, sizeof(number), "%u.%u %%",
                       (unsigned)(sensor_sample.humidity_tenths / 10U),
                       (unsigned)(sensor_sample.humidity_tenths % 10U));
        (void)snprintf(humidity, sizeof(humidity), "%-9s", number);
    }
    const char *status = sensor_failed ? "ERROR   " :
                         dht20_valid ? "OK " : "LOADING...   ";
    return 
        ST7789_Text(20, 10, temperature, ST7789_RED, SCREEN_BG, 2) &&
        ST7789_Text(120, 10, humidity, SCREEN_ACCENT, SCREEN_BG, 2) &&
        // ST7789_Text(160, 10, status, sensor_failed ? ST7789_RED : ST7789_WHITE, SCREEN_BG, 1) &&
        ST7789_Text(80, 60, pressed ? "00.0 V" : "00.0 A",
                    pressed ? ST7789_GREEN : ST7789_WHITE, SCREEN_BG, 4);
}

static bool display_init(void)
{
    if (!Board_TFT_Init() || !ST7789_Init(Board_TFT_Write, Board_TFT_Delay) ||
        !ST7789_Fill(SCREEN_BG) ||
        // !ST7789_Text(30, 12, "My Multimeter", SCREEN_ACCENT, SCREEN_BG, 3) ||
        // !ST7789_Text(20, 20, "00.0", ST7789_WHITE, SCREEN_BG, 4)
        
        // !ST7789_FillRect(0, 235, 240, 5, ST7789_WHITE) ||
        // !ST7789_FillRect(0, 0, 5, 240, ST7789_WHITE) ||
        // !ST7789_FillRect(235, 0, 5, 240, ST7789_WHITE) ||

        !ST7789_Text(20, 50, "DC", SCREEN_ACCENT, SCREEN_BG, 2) ||
        !ST7789_FillRect(20, 72, 30, 2, ST7789_WHITE) ||
        !ST7789_Text(20, 80, "AC", SCREEN_ACCENT, SCREEN_BG, 2)
        )
        return false;
    if (!display_status(button_pressed)) return false;
    displayed_sensor_revision = sensor_revision;
    displayed_button = button_pressed;
    return true;
}

void app_init(void)
{
    led_init();
    button_pressed = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET;
    tft_ready = display_init();
    last_tft_attempt = HAL_GetTick();
#if APP_ENABLE_OLED
    oled_ready = oled_init();
    last_oled_attempt = HAL_GetTick();
#endif
    last_toggle_tick = HAL_GetTick();
    last_sensor_attempt = HAL_GetTick() - 2000U;
}

void app_process(void)
{
    uint32_t now = HAL_GetTick();
    button_pressed = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET;
    elapsed_ms = now - last_toggle_tick;
    if (elapsed_ms >= 500U) {
        led_toggle();
        last_toggle_tick = now;
    }

    sensor_process(HAL_GetTick());
    now = HAL_GetTick();
    if (!tft_ready) {
        if ((uint32_t)(now - last_tft_attempt) >= 1000U) {
            tft_ready = display_init();
            last_tft_attempt = HAL_GetTick();
        }
    } else if (sensor_revision != displayed_sensor_revision || button_pressed != displayed_button) {
        tft_ready = display_status(button_pressed);
        if (tft_ready) {
            displayed_sensor_revision = sensor_revision;
            displayed_button = button_pressed;
        }
        last_tft_attempt = HAL_GetTick();
    }

#if APP_ENABLE_OLED
    /* Run independently of TFT success; bound redraws to at most 10 Hz.
     * A full I2C frame takes about 47 ms at 100 kHz. */
    now = HAL_GetTick();
    if (!oled_ready) {
        if ((uint32_t)(now - last_oled_attempt) >= 1000U) {
            oled_ready = oled_init();
            last_oled_attempt = HAL_GetTick();
        }
    } else if ((uint32_t)(now - last_oled_attempt) >= 100U &&
               (now / 1000U != oled_displayed_seconds || button_pressed != oled_displayed_button)) {
        oled_ready = oled_status(now / 1000U, button_pressed);
        last_oled_attempt = HAL_GetTick();
    }
#endif
}
