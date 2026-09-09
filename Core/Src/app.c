#include "app.h"
#include "led.h"
#include "main.h"
#include "board_tft.h"
#include "st7789.h"
#include <stdio.h>

#define SCREEN_BG 0x0863U
#define SCREEN_ACCENT 0x07FFU

static uint32_t last_toggle_tick;
static uint32_t last_tft_attempt;
static uint32_t displayed_seconds;
static uint8_t displayed_button;

volatile uint32_t elapsed_ms;
volatile uint8_t button_pressed;
/* Transport success only: a write-only SPI screen cannot acknowledge presence. */
volatile uint8_t tft_ready;

static bool display_status(uint32_t seconds, uint8_t pressed)
{
    /* Fixed width clears old digits without redrawing the whole screen. */
    char uptime[20];
    (void)snprintf(uptime, sizeof(uptime), "%7lu S", (unsigned long)seconds);
    return ST7789_Text(16, 106, uptime, ST7789_WHITE, SCREEN_BG, 3) &&
        ST7789_Text(16, 164, pressed ? "DANG NHAN" : "DA THA   ",
                    pressed ? ST7789_GREEN : ST7789_WHITE, SCREEN_BG, 3);
}

static bool display_init(void)
{
    if (!Board_TFT_Init() || !ST7789_Init(Board_TFT_Write, Board_TFT_Delay) ||
        !ST7789_Fill(SCREEN_BG) ||
        !ST7789_Text(30, 12, "XIN CHAO!", SCREEN_ACCENT, SCREEN_BG, 3) ||
        !ST7789_Text(12, 44, "STM32F411 TFT 240X240", ST7789_WHITE, SCREEN_BG, 1) ||
        !ST7789_FillRect(12, 64, 72, 5, ST7789_RED) ||
        !ST7789_FillRect(84, 64, 72, 5, ST7789_GREEN) ||
        !ST7789_FillRect(156, 64, 72, 5, ST7789_BLUE) ||
        !ST7789_Text(16, 82, "THOI GIAN CHAY", SCREEN_ACCENT, SCREEN_BG, 2) ||
        !ST7789_Text(16, 142, "NUT PA0", SCREEN_ACCENT, SCREEN_BG, 2) ||
        !ST7789_Text(16, 212, "NHAN NUT DE THU", ST7789_WHITE, SCREEN_BG, 2))
        return false;
    uint32_t seconds = HAL_GetTick() / 1000U;
    if (!display_status(seconds, button_pressed)) return false;
    displayed_seconds = seconds;
    displayed_button = button_pressed;
    return true;
}

void app_init(void)
{
    led_init();
    button_pressed = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET;
    tft_ready = display_init();
    last_tft_attempt = HAL_GetTick();
    last_toggle_tick = HAL_GetTick();
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

    if (!tft_ready) {
        if ((uint32_t)(now - last_tft_attempt) >= 1000U) {
            tft_ready = display_init();
            last_tft_attempt = HAL_GetTick();
        }
    } else if (now / 1000U != displayed_seconds || button_pressed != displayed_button) {
        uint32_t seconds = now / 1000U;
        tft_ready = display_status(seconds, button_pressed);
        if (tft_ready) {
            displayed_seconds = seconds;
            displayed_button = button_pressed;
        }
        last_tft_attempt = HAL_GetTick();
    }
}
