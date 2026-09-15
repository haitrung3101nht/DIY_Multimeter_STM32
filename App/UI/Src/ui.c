#include "ui_screen.h"
#include "tft.h"

/* Add a screen here to register its navigation key. First entry is home. */
static const UI_Screen * const screens[] = {
    &screen_function0,
    &screen_function1,
    &screen_function2,
    &screen_function3,
    // &screen_function4,
};
static const UI_Screen *current;
void UI_Text(uint8_t id, uint16_t x, uint16_t y, const char *text, uint16_t color)
{
    TFT_SetText(id, x, y, text, color);
}

static void UI_Open(const UI_Screen *screen, const UI_Model *model)
{
    current = screen;
    TFT_ClearScene();
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
