/* Coordinates hardware, timekeeping and UI; retries display failures. */
#include "app.h"
#include "board_oled.h"
#include "clock_time.h"
#include "clock_view.h"
#include "ssd1306.h"
#include "main.h"
static ClockTime clock;
static bool ready;
static uint32_t retry_tick;
static uint32_t shown_seconds;
static bool WriteDisplay(uint8_t control, const uint8_t *data, uint16_t size)
{
    return Board_OLED_Write(control, data, size) == HAL_OK;
}
static bool StartDisplay(void)
{
    return Board_OLED_Init() == HAL_OK && SSD1306_Init(WriteDisplay);
}
void App_Init(void)
{
    ClockTime_Init(&clock, HAL_GetTick());
    HAL_Delay(100); /* Allow OLED power to settle. */
    ready = StartDisplay();
    retry_tick = HAL_GetTick();
    shown_seconds = UINT32_MAX;
}
void App_Process(void)
{
    uint32_t now = HAL_GetTick();
    Value_set(text);
    // ClockTime_Update(&clock, now);
    // if (!ready && (uint32_t)(now - retry_tick) >= 1000U) {
    //     ready = StartDisplay();
    //     retry_tick = HAL_GetTick();
    //     shown_seconds = UINT32_MAX;
    // }
    // if (ready && shown_seconds != clock.seconds) {
    //     char text[9];
    //     // ClockTime_Format(&clock, text);
    //     // ready = ClockView_Show(text);
    //     ready = Value_set(text);
    //     if (ready) shown_seconds = clock.seconds;
    //     else retry_tick = HAL_GetTick();
    // }
    /* Black Pill PC13 LED is active low: lit when OLED communication fails. */
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, ready ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
