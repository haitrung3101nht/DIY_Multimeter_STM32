#ifndef UI_SCREEN_H
#define UI_SCREEN_H
#include "ui.h"
/* Screens only enqueue drawing; never wait or read hardware here. */
typedef struct {
    char key;
    void (*render)(const UI_Model *model);
} UI_Screen;
void UI_Text(uint8_t id, uint16_t x, uint16_t y, const char *text, uint16_t color);
extern const UI_Screen screen_function0;
extern const UI_Screen screen_function1;
extern const UI_Screen screen_function2;
extern const UI_Screen screen_function3;
// extern const UI_Screen screen_function4;
#endif
