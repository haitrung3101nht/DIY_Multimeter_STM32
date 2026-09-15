#include "ui_screen.h"
#include "tft.h"
#include <stdio.h>

static void value_row(uint8_t row, float value, const char *unit, uint16_t color)
{
    char text[24];
    int v = (int)(value * 10.0f + (value >= 0 ? 0.5f : -0.5f));
    unsigned magnitude = (unsigned)(v < 0 ? -v : v);
    snprintf(text, sizeof(text), "%s%u.%u %s", v < 0 ? "-" : "",
             magnitude / 10U, magnitude % 10U, unit);
    UI_Row(row, text, color);
}

static void render(const UI_Model *model)
{
    UI_Row(0, "TEMPERATURE", TFT_WHITE);
    UI_Row(2, "HUMIDITY", TFT_GREEN);
    if (model->sensor_valid) {
        value_row(1, model->temperature, "C", TFT_WHITE);
        value_row(3, model->humidity, "%RH", TFT_GREEN);
    } else {
        UI_Row(1, "--.- C", TFT_RED);
        UI_Row(3, "--.- %RH", TFT_RED);
    }
    UI_Row(4, "0:HOME 4:ABC", TFT_WHITE);
    UI_Row(5, "8:F3 A:F4", TFT_WHITE);
}
const UI_Screen screen_climate = {'0', render};
