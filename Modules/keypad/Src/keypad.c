#include "keypad.h"
#include "main.h"

#define KEYPAD_DEBOUNCE_MS  20U
#define KEYPAD_MULTIPLE    '\x7F'

static GPIO_TypeDef * const row_ports[4] = {
    GPIOA, GPIOA, GPIOA, GPIOA
};

static const uint16_t row_pins[4] = {
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3
};

static GPIO_TypeDef * const col_ports[4] = {
    GPIOB, GPIOB, GPIOB, GPIOB
};

static const uint16_t col_pins[4] = {
    GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15
};

static const char keymap[4][4] = {
    {'0', '1', '2', '3'},
    {'4', '5', '6', '7'},
    {'8', '9', '*', '#'},
    {'A', 'B', 'C', 'D'}
};

static char candidate;
static uint32_t candidate_since;
static uint8_t armed;

/*
 * Chờ khoảng 5 us cho mức điện áp cột ổn định.
 * Đây là vòng chờ ngắn, không dùng HAL_Delay(1) cho mỗi hàng.
 */
static void Keypad_Settle(void)
{
    uint32_t loops = SystemCoreClock / 1000000U * 5U;

    for (volatile uint32_t i = 0; i < loops; i++)
    {
        __NOP();
    }
}

static char Keypad_ScanRaw(void)
{
    char result = '\0';
    uint8_t pressed_count = 0;

    for (uint8_t row = 0; row < 4U; row++)
    {
        HAL_GPIO_WritePin(row_ports[row],
                          row_pins[row], GPIO_PIN_SET);
    }

    for (uint8_t row = 0; row < 4U; row++)
    {
        HAL_GPIO_WritePin(row_ports[row],
                          row_pins[row], GPIO_PIN_RESET);

        Keypad_Settle();

        for (uint8_t col = 0; col < 4U; col++)
        {
            if (HAL_GPIO_ReadPin(col_ports[col],
                                 col_pins[col]) == GPIO_PIN_RESET)
            {
                result = keymap[row][col];
                pressed_count++;
            }
        }

        HAL_GPIO_WritePin(row_ports[row],
                          row_pins[row], GPIO_PIN_SET);
    }

    if (pressed_count > 1U)
        return KEYPAD_MULTIPLE;

    return result;
}

void Keypad_Init(void)
{
    /* This module configures its pins; no CubeMX keypad setup is required. */
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    for (uint8_t row = 0; row < 4U; row++)
    {
        HAL_GPIO_WritePin(row_ports[row],
                          row_pins[row], GPIO_PIN_SET);
    }

    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &gpio);

    candidate = '\0';
    candidate_since = HAL_GetTick();
    armed = 1;
}

char Keypad_GetKey(void)
{
    char raw = Keypad_ScanRaw();
    uint32_t now = HAL_GetTick();

    if (raw == KEYPAD_MULTIPLE)
    {
        armed = 0;
    }

    if (raw != candidate)
    {
        candidate = raw;
        candidate_since = now;
        return '\0';
    }

    if ((uint32_t)(now - candidate_since) < KEYPAD_DEBOUNCE_MS)
        return '\0';

    if (candidate == '\0')
    {
        /* Chỉ cho phép lần nhấn tiếp theo sau khi thả ổn định. */
        armed = 1;
        return '\0';
    }

    if (candidate == KEYPAD_MULTIPLE || !armed)
        return '\0';

    armed = 0;
    return candidate;
}
