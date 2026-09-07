/* Pure timekeeping logic; no STM32 or display dependency. */
#include "clock_time.h"
void ClockTime_Init(ClockTime *clock, uint32_t now)
{
    clock->last_tick = now;
    clock->seconds = 0;
}
void ClockTime_Update(ClockTime *clock, uint32_t now)
{
    /* Unsigned subtraction handles HAL_GetTick rollover. Call at least
       once per 2^32 milliseconds. Preserve fractional milliseconds. */
    uint32_t elapsed = (uint32_t)(now - clock->last_tick) / 1000U;
    clock->last_tick += elapsed * 1000U;
    clock->seconds = (clock->seconds + elapsed) % 86400U;
}
void ClockTime_Format(const ClockTime *clock, char text[9])
{
    uint32_t hours = clock->seconds / 3600U;
    uint32_t minutes = clock->seconds / 60U % 60U;
    uint32_t seconds = clock->seconds % 60U;
    text[0] = '0' + hours / 10U; text[1] = '0' + hours % 10U;
    text[2] = ':';
    text[3] = '0' + minutes / 10U; text[4] = '0' + minutes % 10U;
    text[5] = ':';
    text[6] = '0' + seconds / 10U; text[7] = '0' + seconds % 10U;
    text[8] = '\0';
}

void Value_set(char text[12])
{
    text[0] = 'H';
    text[1] = 'e';
    text[2] = 'l';
    text[3] = 'l';
    text[4] = 'o';
    text[5] = ' ';
    text[6] = 'W';
    text[7] = 'o';
    text[8] = 'r';
    text[9] = 'l';
    text[10] = 'd';
    text[11] = '\0';
}