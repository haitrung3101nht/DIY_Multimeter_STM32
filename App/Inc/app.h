#ifndef APP_H
#define APP_H

/* Call once after HAL, system clock, GPIO and SPI initialization. */
void App_Init(void);

/* Call repeatedly from the main loop. */
void App_Process(void);

#endif
