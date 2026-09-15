#ifndef UI_H
#define UI_H
#include <stdint.h>
#include <stdbool.h>
/* Application data belongs to the app, independently of the visible screen. */
typedef struct {
    float temperature;
    float humidity;
    bool sensor_valid;
} UI_Model;
void UI_Init(const UI_Model *model);
void UI_HandleKey(char key, const UI_Model *model);
void UI_DataChanged(const UI_Model *model);
#endif
