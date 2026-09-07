#include "app.h"
#include "led.h"
#include "main.h"

#define LESSON_ON_OFF  1
#define LESSON_TOGGLE  2
#define LESSON_BUTTON  3

/* Change this value to select the exercise, then rebuild and flash. */
#ifndef APP_LESSON
#define APP_LESSON LESSON_TOGGLE
#endif

void app_init(void)
{
    led_init();
}

void app_process(void)
{
#if APP_LESSON == LESSON_ON_OFF
    led_on();
    HAL_Delay(500);
    led_off();
    HAL_Delay(500);
#elif APP_LESSON == LESSON_TOGGLE
    led_toggle();
    HAL_Delay(500);
#elif APP_LESSON == LESSON_BUTTON
    /* Pull-up input: released = SET, pressed = RESET. */
    if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET) {
        led_on();
    } else {
        led_off();
    }
#else
#error "Invalid APP_LESSON"
#endif
}
