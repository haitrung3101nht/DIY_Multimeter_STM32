#include "ui_screen.h"
#include "tft.h"
static void render(const UI_Model *model)
{
    (void)model;
    UI_Row(0, "ALPHABET", TFT_GREEN);
    UI_Row(1, "ABCDEFGHIJKLM", TFT_WHITE);
    UI_Row(2, "NOPQRSTUVWXYZ", TFT_WHITE);
    UI_Row(3, "abcdefghijklm", TFT_WHITE);
    UI_Row(4, "nopqrstuvwxyz", TFT_WHITE);
    UI_Row(5, "0  4  8  A", TFT_GREEN);
}
const UI_Screen screen_alphabet = {'4', render};
