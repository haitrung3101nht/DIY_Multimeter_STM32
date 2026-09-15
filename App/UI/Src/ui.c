#include "ui_screen.h"
#include "tft.h"

/* Add a screen here to register its navigation key. First entry is home. */
static const UI_Screen * const screens[] = {
    &screen_climate,
    &screen_alphabet,
    &screen_function3,
    &screen_function4,
};
static const UI_Screen *current;
static const uint16_t row_y[] = {8, 40, 80, 112, 152, 184};

void UI_Row(uint8_t row, const char *text, uint16_t color)
{
    if (row < sizeof(row_y) / sizeof(row_y[0]))
        TFT_SetText(row, row_y[row], text, color);
}

static void UI_Open(const UI_Screen *screen, const UI_Model *model)
{
    current = screen;
    /* Cancel any old scanline job, then replace all six fields. */
    TFT_InvalidateText();
    for (uint8_t row = 0; row < 6; ++row)
        UI_Row(row, "", TFT_WHITE);
    current->render(model);
}

void UI_Init(const UI_Model *model)
{
    UI_Open(screens[0], model);
}

void UI_HandleKey(char key, const UI_Model *model)
{
    for (unsigned i = 0; i < sizeof(screens) / sizeof(screens[0]); ++i) {
        if (screens[i]->key != key) continue;
        if (current != screens[i]) UI_Open(screens[i], model);
        return;
    }
    /* Unassigned keys have no action. */
}

void UI_DataChanged(const UI_Model *model)
{
    if (current) current->render(model);
}
