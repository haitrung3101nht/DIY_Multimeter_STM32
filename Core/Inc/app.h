#ifndef APP_H
#define APP_H

/* Set to 1 to restore the SSD1306 alongside the TFT. */
#ifndef APP_ENABLE_OLED
#define APP_ENABLE_OLED 0
#endif

void app_init(void);
void app_process(void);

#endif
