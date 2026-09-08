#include "app.h"
#include "led.h"
#include "main.h"

static uint32_t last_toggle_tick;

volatile uint32_t elapsed_ms;
volatile uint8_t button_pressed;

void app_init(void)
{
    led_init();
    last_toggle_tick = HAL_GetTick();
}

void app_process(void)
{
    uint32_t now = HAL_GetTick();

    /* Đọc button mỗi vòng lặp: nhấn = 1, thả = 0. */
    button_pressed =
        (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin)
         == GPIO_PIN_RESET);

    /* Tính thời gian từ lần toggle trước. */
    elapsed_ms = now - last_toggle_tick;

    if (elapsed_ms >= 500U) {
        led_toggle();
        last_toggle_tick = now;
    }
}