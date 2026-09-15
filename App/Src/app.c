#include "app.h"
#include "main.h"
#include "tft.h"
#include "dht20.h"
#include <stdio.h>

static uint8_t sensor_ready;
static uint32_t last_measurement;
static uint32_t last_led;

/*
 * Chuyển số thực thành chuỗi có một chữ số thập phân.
 * Không cần bật hỗ trợ printf float trong linker.
 */
static void App_DrawValue(uint16_t y, float value, uint16_t color)
{
    char text[16];

    int value10 = (int)(value * 10.0f +
                       (value >= 0.0f ? 0.5f : -0.5f));

    unsigned int magnitude =
        (unsigned int)(value10 < 0 ? -value10 : value10);

    snprintf(text, sizeof(text), "%s%u.%u",
             value10 < 0 ? "-" : "",
             magnitude / 10U,
             magnitude % 10U);

    TFT_FillRect(20, y, 200, 32, TFT_BLACK);
    TFT_DrawString(20, y, text, color, TFT_BLACK, 4);
}

void App_Init(void)
{
    TFT_Init();

    TFT_DrawString(20, 30, "-----",
                   TFT_WHITE, TFT_BLACK, 4);
    TFT_DrawString(20, 90, "-----",
                   TFT_GREEN, TFT_BLACK, 4);

    sensor_ready = (DHT20_Init() == DHT20_OK);

    last_led = HAL_GetTick();

    /* Cho phép đo ngay trong lần App_Process đầu tiên. */
    last_measurement = HAL_GetTick() - 2000U;
}

void App_Process(void)
{
    uint32_t now = HAL_GetTick();

    if ((uint32_t)(now - last_led) >= 500U)
    {
        last_led = now;
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    }

    if ((uint32_t)(now - last_measurement) < 2000U)
        return;

    last_measurement = now;

    if (!sensor_ready)
        sensor_ready = (DHT20_Init() == DHT20_OK);

    DHT20_Data data;

    if (sensor_ready && DHT20_Read(&data) == DHT20_OK)
    {
        App_DrawValue(30, data.temperature, TFT_WHITE);
        App_DrawValue(90, data.humidity, TFT_GREEN);
    }
    else
    {
        sensor_ready = 0;

        TFT_FillRect(20, 30, 200, 32, TFT_BLACK);
        TFT_FillRect(20, 90, 200, 32, TFT_BLACK);

        TFT_DrawString(20, 30, "-----",
                       TFT_RED, TFT_BLACK, 4);
        TFT_DrawString(20, 90, "-----",
                       TFT_RED, TFT_BLACK, 4);
    }
}