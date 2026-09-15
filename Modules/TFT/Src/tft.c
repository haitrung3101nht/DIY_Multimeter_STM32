#include "tft.h"
#include "main.h"
#include "spi.h"

#define TFT_WIDTH   240U
#define TFT_HEIGHT  240U

/*
 * Landscape 90 degrees: MADCTL = 0x60, offsets = 0.
 */
#define TFT_X_OFFSET  0U
#define TFT_Y_OFFSET  0U

static const uint8_t digit_font[10][7] = {
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, /* 0 */
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, /* 1 */
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, /* 2 */
    {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E}, /* 3 */
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, /* 4 */
    {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E}, /* 5 */
    {0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E}, /* 6 */
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, /* 7 */
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, /* 8 */
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E}  /* 9 */
};


static void TFT_Send(uint8_t *data, uint16_t size)
{
    if (HAL_SPI_Transmit(&hspi1, data, size, 5) != HAL_OK)
    {
        Error_Handler();
    }
}

static void TFT_Command(uint8_t command)
{
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port,
                      TFT_DC_Pin, GPIO_PIN_RESET);

    TFT_Send(&command, 1);
}

static void TFT_Data(uint8_t *data, uint16_t size)
{
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port,
                      TFT_DC_Pin, GPIO_PIN_SET);

    TFT_Send(data, size);
}

static void TFT_SetWindow(uint16_t x0, uint16_t y0,
                          uint16_t x1, uint16_t y1)
{
    x0 += TFT_X_OFFSET;
    x1 += TFT_X_OFFSET;
    y0 += TFT_Y_OFFSET;
    y1 += TFT_Y_OFFSET;

    uint8_t columns[4] = {
        (uint8_t)(x0 >> 8), (uint8_t)x0,
        (uint8_t)(x1 >> 8), (uint8_t)x1
    };

    uint8_t rows[4] = {
        (uint8_t)(y0 >> 8), (uint8_t)y0,
        (uint8_t)(y1 >> 8), (uint8_t)y1
    };

    TFT_Command(0x2A); /* CASET: chọn cột */
    TFT_Data(columns, sizeof(columns));

    TFT_Command(0x2B); /* RASET: chọn hàng */
    TFT_Data(rows, sizeof(rows));

    TFT_Command(0x2C); /* RAMWR: bắt đầu ghi pixel */
}

void TFT_Init(void)
{
    uint8_t value;

    /* Reset phần cứng sau khi nguồn ổn định. */
    HAL_GPIO_WritePin(TFT_RST_GPIO_Port,
                      TFT_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(20);

    HAL_GPIO_WritePin(TFT_RST_GPIO_Port,
                      TFT_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);

    HAL_GPIO_WritePin(TFT_RST_GPIO_Port,
                      TFT_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(150);

    TFT_Command(0x01); /* Software reset */
    HAL_Delay(150);

    TFT_Command(0x11); /* Sleep out */
    HAL_Delay(120);

    TFT_Command(0x3A); /* Pixel format */
    value = 0x55;      /* RGB565: 16 bit/pixel */
    TFT_Data(&value, 1);
    HAL_Delay(10);

    TFT_Command(0x36); /* Memory access control */
    value = 0x60;
    TFT_Data(&value, 1);

    TFT_Command(0x21); /* Inversion on */
    HAL_Delay(10);

    TFT_Command(0x13); /* Normal display mode */
    HAL_Delay(10);

    /* Xóa RAM hiển thị trước khi bật màn hình. */
    TFT_FillScreen(TFT_BLACK);

    TFT_Command(0x29); /* Display on */
    HAL_Delay(100);
}

void TFT_FillScreen(uint16_t color)
{
    /* Một dòng RGB565 = 240 × 2 = 480 byte. */
    uint8_t line[TFT_WIDTH * 2U];

    for (uint16_t x = 0; x < TFT_WIDTH; x++)
    {
        /* Gửi byte cao trước, byte thấp sau. */
        line[2U * x]      = (uint8_t)(color >> 8);
        line[2U * x + 1U] = (uint8_t)color;
    }

    TFT_SetWindow(0, 0, TFT_WIDTH - 1U, TFT_HEIGHT - 1U);

    for (uint16_t y = 0; y < TFT_HEIGHT; y++)
    {
        TFT_Data(line, sizeof(line));
    }

}


void TFT_FillRect(uint16_t x, uint16_t y,
                  uint16_t width, uint16_t height,
                  uint16_t color)
{
    if (x >= TFT_WIDTH || y >= TFT_HEIGHT ||
        width == 0U || height == 0U)
    {
        return;
    }

    /* Cắt phần hình chữ nhật vượt ra ngoài màn hình. */
    if (width > TFT_WIDTH - x)
        width = TFT_WIDTH - x;

    if (height > TFT_HEIGHT - y)
        height = TFT_HEIGHT - y;

    uint8_t line[TFT_WIDTH * 2U];

    for (uint16_t i = 0; i < width; i++)
    {
        line[2U * i]      = (uint8_t)(color >> 8);
        line[2U * i + 1U] = (uint8_t)color;
    }

    TFT_SetWindow(x, y, x + width - 1U, y + height - 1U);

    for (uint16_t row = 0; row < height; row++)
    {
        TFT_Data(line, (uint16_t)(width * 2U));
    }
}

void TFT_DrawChar(uint16_t x, uint16_t y, char ch,
                  uint16_t color, uint16_t background,
                  uint8_t scale)
{
    if (scale == 0U || x >= TFT_WIDTH || y >= TFT_HEIGHT)
        return;

    /*
     * Một ô ký tự: 5 cột chữ + 1 cột cách,
     *               7 hàng chữ + 1 hàng cách.
     */
    TFT_FillRect(x, y, 6U * scale, 8U * scale, background);

    for (uint8_t row = 0; row < 7U; row++)
    {
        uint8_t bits = 0;

        if (ch >= '0' && ch <= '9')
        {
            bits = digit_font[ch - '0'][row];
        }
        else if (ch == '-' && row == 3U)
        {
            bits = 0x1F;
        }
        else if (ch == '.' && row == 6U)
        {
            bits = 0x04;
        }

        /* Khoảng trắng và ký tự chưa hỗ trợ chỉ vẽ nền. */
        for (uint8_t col = 0; col < 5U; col++)
        {
            if (bits & (1U << (4U - col)))
            {
                TFT_FillRect(x + col * scale,
                             y + row * scale,
                             scale, scale, color);
            }
        }
    }
}

void TFT_DrawString(uint16_t x, uint16_t y, const char *text,
                    uint16_t color, uint16_t background,
                    uint8_t scale)
{
    if (text == 0 || scale == 0U ||
        x >= TFT_WIDTH || y >= TFT_HEIGHT)
    {
        return;
    }

    uint16_t advance = 6U * scale;

    while (*text != '\0')
    {
        /* Dừng khi không còn đủ chỗ cho một ô ký tự. */
        if (advance > TFT_WIDTH - x)
            break;

        TFT_DrawChar(x, y, *text, color, background, scale);

        x += advance;
        text++;
    }
}
/* Retained text elements with pixel coordinates and dirty-scanline rendering. */
#include <string.h>
#include "font_smooth.h"
#define FIELD_COUNT 16U
#define FIELD_CHARS 31U
#define FONT_W 16U
#define FONT_H 28U

typedef struct {
    char text[FIELD_CHARS + 1U];
    uint16_t x, y, color;
    uint8_t used;
} TextElement;
static TextElement elements[FIELD_COUNT];
static uint8_t dirty_rows[TFT_HEIGHT];
static uint16_t next_row;

static void mark_rows(uint16_t y)
{
    for (unsigned row = y; row < TFT_HEIGHT && row < y + FONT_H; ++row)
        dirty_rows[row] = 1;
}

void TFT_SetText(uint8_t id, uint16_t x, uint16_t y,
                 const char *text, uint16_t color)
{
    if (id >= FIELD_COUNT || x >= TFT_WIDTH || y >= TFT_HEIGHT || !text) return;
    TextElement *e = &elements[id];
    char value[FIELD_CHARS + 1U] = {0};
    for (unsigned i = 0; i < FIELD_CHARS && text[i]; ++i) value[i] = text[i];
    if (e->used && e->x == x && e->y == y && e->color == color &&
        strcmp(e->text, value) == 0) return;
    if (e->used) mark_rows(e->y);
    memcpy(e->text, value, sizeof(value));
    e->x = x;
    e->y = y;
    e->color = color;
    e->used = 1;
    mark_rows(y);
}

void TFT_RemoveText(uint8_t id)
{
    if (id >= FIELD_COUNT || !elements[id].used) return;
    mark_rows(elements[id].y);
    elements[id].used = 0;
}

/* Clear the scene in RAM; display clearing is spread over main-loop iterations. */
void TFT_ClearScene(void)
{
    memset(elements, 0, sizeof(elements));
    memset(dirty_rows, 1, sizeof(dirty_rows));
    next_row = 0;
}

static uint16_t blend(uint16_t fg, uint16_t bg, unsigned a)
{
    unsigned inv = 255U - a;
    unsigned r = (((fg >> 11) & 31U) * a + ((bg >> 11) & 31U) * inv + 127U) / 255U;
    unsigned g = (((fg >> 5) & 63U) * a + ((bg >> 5) & 63U) * inv + 127U) / 255U;
    unsigned b = ((fg & 31U) * a + (bg & 31U) * inv + 127U) / 255U;
    return (uint16_t)((r << 11) | (g << 5) | b);
}

void TFT_Process(void)
{
    unsigned y;
    for (unsigned n = 0; n < TFT_HEIGHT; ++n) {
        y = (next_row + n) % TFT_HEIGHT;
        if (dirty_rows[y]) goto render;
    }
    return;
render:
    dirty_rows[y] = 0;
    next_row = (uint16_t)((y + 1U) % TFT_HEIGHT);
    /* Recompose the full row, restoring other elements under moved/deleted text. */
    uint8_t pixels[TFT_WIDTH * 2U] = {0};
    for (unsigned id = 0; id < FIELD_COUNT; ++id) {
        const TextElement *e = &elements[id];
        if (!e->used || y < e->y || y >= e->y + FONT_H) continue;
        unsigned font_row = (y - e->y) * FONT_W;
        for (unsigned c = 0; c < FIELD_CHARS && e->text[c]; ++c) {
            unsigned start = e->x + c * FONT_W;
            if (start >= TFT_WIDTH) break;
            unsigned char ch = (unsigned char)e->text[c];
            if (ch < 32 || ch > 126) ch = '?';
            for (unsigned x = 0; x < FONT_W && start + x < TFT_WIDTH; ++x) {
                unsigned a = smooth_font[ch - 32][font_row + x];
                if (!a) continue;
                unsigned index = (start + x) * 2U;
                uint16_t bg = (uint16_t)((pixels[index] << 8) | pixels[index + 1]);
                uint16_t color = blend(e->color, bg, a);
                pixels[index] = (uint8_t)(color >> 8);
                pixels[index + 1] = (uint8_t)color;
            }
        }
    }
    TFT_SetWindow(0, (uint16_t)y, TFT_WIDTH - 1U, (uint16_t)y);
    TFT_Data(pixels, sizeof(pixels));
}
