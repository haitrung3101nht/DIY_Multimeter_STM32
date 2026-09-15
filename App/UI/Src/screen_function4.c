#include "ui_screen.h"
#include "tft.h"
/* Replace this render function with the feature you want. */
static void render(const UI_Model *model)
{
    (void)model;
    UI_Row(0, "FUNCTION 4", TFT_GREEN);
    UI_Row(1, "NOT SET YET", TFT_WHITE);
    UI_Row(2, "", TFT_WHITE);
    UI_Row(3, "", TFT_WHITE);
    UI_Row(4, "EDIT SCREEN", TFT_WHITE);
    UI_Row(5, "0  4  8  A", TFT_GREEN);
}
const UI_Screen screen_function4 = {'A', render};
