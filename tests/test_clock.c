#include "clock_time.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
static void expect(ClockTime *c, const char *expected) {
    char text[9]; ClockTime_Format(c, text); assert(strcmp(text, expected) == 0);
}
int main(void) {
    ClockTime c;
    ClockTime_Init(&c, 100);
    ClockTime_Update(&c, 1099); expect(&c, "00:00:00");
    ClockTime_Update(&c, 1100); expect(&c, "00:00:01");
    ClockTime_Update(&c, 60099); expect(&c, "00:00:59");
    ClockTime_Update(&c, 60100); expect(&c, "00:01:00");
    ClockTime_Update(&c, 3600100); expect(&c, "01:00:00");
    ClockTime_Update(&c, 86400100); expect(&c, "00:00:00");
    ClockTime_Init(&c, UINT32_MAX - 499U);
    ClockTime_Update(&c, 499); expect(&c, "00:00:00");
    ClockTime_Update(&c, 500); expect(&c, "00:00:01");
    ClockTime_Update(&c, 3750); expect(&c, "00:00:04");
    ClockTime_Update(&c, 4500); expect(&c, "00:00:05");
    puts("Clock tests passed");
}
