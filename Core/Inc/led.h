#ifndef LED_H
#define LED_H

/* Call after MX_GPIO_Init(). */
void led_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);

#endif
