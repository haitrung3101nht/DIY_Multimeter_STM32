#include "app.h"
#include "main.h"
#include "tft.h"
#include "dht20.h"
#include "keypad.h"
#include "ui.h"

static uint32_t last_led, last_key_scan, last_measurement, phase_started, last_poll;
static enum { SENSOR_IDLE, SENSOR_READY_WAIT, SENSOR_MEASURING } sensor_phase;

static UI_Model model;

static void App_SensorError(void)
{
    model.sensor_valid = false;
    UI_DataChanged(&model);
    sensor_phase = SENSOR_IDLE;
}

static void App_SensorTask(uint32_t now)
{
    if (sensor_phase == SENSOR_IDLE) {
        if ((uint32_t)(now - last_measurement) < 2000U) return;
        last_measurement = now;
        if (DHT20_CheckReady() != DHT20_OK) {
            App_SensorError();
            return;
        }
        phase_started = HAL_GetTick();
        sensor_phase = SENSOR_READY_WAIT;
        return;
    }
    if (sensor_phase == SENSOR_READY_WAIT) {
        if ((uint32_t)(now - phase_started) < 10U) return;
        if (DHT20_StartMeasurement() != DHT20_OK) {
            App_SensorError();
            return;
        }
        phase_started = HAL_GetTick();
        last_poll = phase_started;
        sensor_phase = SENSOR_MEASURING;
        return;
    }
    uint32_t elapsed = (uint32_t)(now - phase_started);
    if (elapsed >= 200U) {
        App_SensorError();
        return;
    }
    if (elapsed < 85U || (uint32_t)(now - last_poll) < 10U) return;
    last_poll = now;
    DHT20_Data data;
    DHT20_Status status = DHT20_ReadResult(&data);
    if (status == DHT20_BUSY) return;
    if (status != DHT20_OK) {
        App_SensorError();
        return;
    }
    model.temperature = data.temperature;
    model.humidity = data.humidity;
    model.sensor_valid = true;
    UI_DataChanged(&model);
    sensor_phase = SENSOR_IDLE;
}

void App_Init(void)
{
    Keypad_Init();
    TFT_Init(); /* Startup delays only; sensor has also had time to power up. */
    model = (UI_Model){0};
    UI_Init(&model);
    uint32_t now = HAL_GetTick();
    last_led = now;
    last_key_scan = now - 2U;
    last_measurement = now - 2000U;
    sensor_phase = SENSOR_IDLE;
}

void App_Process(void)
{
    uint32_t now = HAL_GetTick();
    if ((uint32_t)(now - last_key_scan) >= 2U) {
        last_key_scan = now;
        char key = Keypad_GetKey();
        if (key) UI_HandleKey(key, &model);
    }
    if ((uint32_t)(now - last_led) >= 500U) {
        last_led = now;
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    }
    App_SensorTask(now);
    TFT_Process(); /* One short scanline; never redraw a whole field here. */
}
