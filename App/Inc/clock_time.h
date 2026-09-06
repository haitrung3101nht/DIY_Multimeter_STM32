#ifndef CLOCK_TIME_H
#define CLOCK_TIME_H
#include <stdint.h>
typedef struct {
    uint32_t last_tick;
    uint32_t seconds;
} ClockTime;
void ClockTime_Init(ClockTime *clock, uint32_t now);
void ClockTime_Update(ClockTime *clock, uint32_t now);
void ClockTime_Format(const ClockTime *clock, char text[9]);
#endif
