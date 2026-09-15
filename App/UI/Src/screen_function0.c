#include "ui_screen.h"
#include "tft.h"
#include <stdio.h>

static void value_at(uint8_t id, uint16_t x, uint16_t y, float value, const char *unit, uint16_t color)
{
    char text[24];
    int v = (int)(value * 10.0f + (value >= 0 ? 0.5f : -0.5f));
    unsigned magnitude = (unsigned)(v < 0 ? -v : v);
    snprintf(text, sizeof(text), "%s%u.%u %s", v < 0 ? "-" : "",
             magnitude / 10U, magnitude % 10U, unit);
    UI_Text(id, x, y, text, color);
}

static void render(const UI_Model *model)
{
    UI_Text(0, 8, 8, "[ Multimeter ]", TFT_RED);
    UI_Text(1, 10, 80, "TEMP", TFT_WHITE);
    UI_Text(2, 10, 120, "HUMI", TFT_GREEN);
    if (model->sensor_valid) {
        value_at(3, 120, 80, model->temperature, "*C", TFT_WHITE);
        value_at(4, 120, 120, model->humidity, "%", TFT_GREEN);
    } else {
        UI_Text(3, 120, 80, "--.- *C", TFT_RED);
        UI_Text(4, 120, 120, "--.- %", TFT_RED);
    }
    // UI_Text(4, 16, 152, "0:HOME 4:ABC", TFT_WHITE);
    // UI_Text(5, 16, 184, "8:F3 A:F4", TFT_WHITE);
}
const UI_Screen screen_function0 = {'0', render};
