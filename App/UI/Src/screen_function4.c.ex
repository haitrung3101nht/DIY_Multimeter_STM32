#include "ui_screen.h"
#include "tft.h"
/* Replace this render function with the feature you want. */
static void render(const UI_Model *model)
{
    (void)model;
    UI_Text(0, 16, 8, "FUNCTION 4", TFT_GREEN);
    UI_Text(1, 16, 40, "NOT SET YET", TFT_WHITE);
    UI_Text(2, 16, 80, "", TFT_WHITE);
    UI_Text(3, 16, 112, "", TFT_WHITE);
    // UI_Text(4, 16, 152, "EDIT SCREEN", TFT_WHITE);
    // UI_Text(5, 16, 184, "0  4  8  A", TFT_GREEN);
}
const UI_Screen screen_function4 = {'A', render};
