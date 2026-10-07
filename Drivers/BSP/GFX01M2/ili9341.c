/**
 ******************************************************************************
 * @file    ili9341.c
 * @brief   Minimal polling-mode SPI driver for the ILI9341 QVGA TFT display
 *          controller used on the X-NUCLEO-GFX01M2 expansion board.
 ******************************************************************************
 */
#include "ili9341.h"
#include "gfx01m2_conf.h"
#include "gfx_font5x7.h"
#include <stdlib.h>
#include <math.h>

/* ILI9341 command set (only the subset used by this driver) */
#define ILI9341_CMD_SWRESET     0x01U
#define ILI9341_CMD_SLPOUT      0x11U
#define ILI9341_CMD_DISPOFF     0x28U
#define ILI9341_CMD_DISPON      0x29U
#define ILI9341_CMD_CASET       0x2AU
#define ILI9341_CMD_PASET       0x2BU
#define ILI9341_CMD_RAMWR       0x2CU
#define ILI9341_CMD_MADCTL      0x36U
#define ILI9341_CMD_PIXFMT      0x3AU
#define ILI9341_CMD_FRMCTR1     0xB1U
#define ILI9341_CMD_DISCTRL     0xB6U
#define ILI9341_CMD_PWCTR1      0xC0U
#define ILI9341_CMD_PWCTR2      0xC1U
#define ILI9341_CMD_VMCTR1      0xC5U
#define ILI9341_CMD_VMCTR2      0xC7U
#define ILI9341_CMD_PWCTRA      0xCBU
#define ILI9341_CMD_PWCTRB      0xCFU
#define ILI9341_CMD_GAMMASET    0x26U
#define ILI9341_CMD_EN3GAM      0xF2U
#define ILI9341_CMD_PUMPRATIO   0xF7U
#define ILI9341_CMD_TIMCTRA     0xE8U
#define ILI9341_CMD_TIMCTRB     0xEAU
#define ILI9341_CMD_PWRSEQ      0xEDU

static SPI_HandleTypeDef *ili9341_hspi;

static void LCD_CS_Low(void)  { HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); }
static void LCD_CS_High(void) { HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); }

static void LCD_WriteCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
    LCD_CS_Low();
    HAL_SPI_Transmit(ili9341_hspi, &cmd, 1, HAL_MAX_DELAY);
    LCD_CS_High();
}

static void LCD_WriteData(const uint8_t *data, uint16_t size)
{
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
    LCD_CS_Low();
    HAL_SPI_Transmit(ili9341_hspi, (uint8_t *)data, size, HAL_MAX_DELAY);
    LCD_CS_High();
}

static void LCD_WriteDataByte(uint8_t data)
{
    LCD_WriteData(&data, 1);
}

static void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t buf[4];

    buf[0] = (uint8_t)(x0 >> 8); buf[1] = (uint8_t)(x0 & 0xFF);
    buf[2] = (uint8_t)(x1 >> 8); buf[3] = (uint8_t)(x1 & 0xFF);
    LCD_WriteCommand(ILI9341_CMD_CASET);
    LCD_WriteData(buf, 4);

    buf[0] = (uint8_t)(y0 >> 8); buf[1] = (uint8_t)(y0 & 0xFF);
    buf[2] = (uint8_t)(y1 >> 8); buf[3] = (uint8_t)(y1 & 0xFF);
    LCD_WriteCommand(ILI9341_CMD_PASET);
    LCD_WriteData(buf, 4);

    LCD_WriteCommand(ILI9341_CMD_RAMWR);
}

void ILI9341_Init(SPI_HandleTypeDef *hspi)
{
    ili9341_hspi = hspi;

    /* Hardware reset pulse (active low) */
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(120);

    LCD_WriteCommand(ILI9341_CMD_SWRESET);
    HAL_Delay(120);

    LCD_WriteCommand(ILI9341_CMD_DISPOFF);

    /* Manufacturer recommended power-up sequence (see ILI9341 datasheet) */
    LCD_WriteCommand(ILI9341_CMD_PWCTRB);   LCD_WriteDataByte(0x00); LCD_WriteDataByte(0x83); LCD_WriteDataByte(0x30);
    LCD_WriteCommand(ILI9341_CMD_PWRSEQ);   LCD_WriteDataByte(0x64); LCD_WriteDataByte(0x03); LCD_WriteDataByte(0x12); LCD_WriteDataByte(0x81);
    LCD_WriteCommand(ILI9341_CMD_TIMCTRA);  LCD_WriteDataByte(0x85); LCD_WriteDataByte(0x01); LCD_WriteDataByte(0x79);
    LCD_WriteCommand(ILI9341_CMD_PWCTRA);   LCD_WriteDataByte(0x39); LCD_WriteDataByte(0x2C); LCD_WriteDataByte(0x00); LCD_WriteDataByte(0x34); LCD_WriteDataByte(0x02);
    LCD_WriteCommand(ILI9341_CMD_PUMPRATIO);LCD_WriteDataByte(0x20);
    LCD_WriteCommand(ILI9341_CMD_TIMCTRB);  LCD_WriteDataByte(0x00); LCD_WriteDataByte(0x00);

    LCD_WriteCommand(ILI9341_CMD_PWCTR1);   LCD_WriteDataByte(0x26);
    LCD_WriteCommand(ILI9341_CMD_PWCTR2);   LCD_WriteDataByte(0x11);
    LCD_WriteCommand(ILI9341_CMD_VMCTR1);   LCD_WriteDataByte(0x35); LCD_WriteDataByte(0x3E);
    LCD_WriteCommand(ILI9341_CMD_VMCTR2);   LCD_WriteDataByte(0xBE);

    LCD_WriteCommand(ILI9341_CMD_MADCTL);   LCD_WriteDataByte(0x08); /* BGR, portrait, no mirror (was 0x48/MX which mirrored the image horizontally on this panel) */
    LCD_WriteCommand(ILI9341_CMD_PIXFMT);   LCD_WriteDataByte(0x55); /* 16 bits/pixel */

    LCD_WriteCommand(ILI9341_CMD_FRMCTR1);  LCD_WriteDataByte(0x00); LCD_WriteDataByte(0x1B);
    LCD_WriteCommand(ILI9341_CMD_DISCTRL);  LCD_WriteDataByte(0x0A); LCD_WriteDataByte(0xA2);

    LCD_WriteCommand(ILI9341_CMD_EN3GAM);   LCD_WriteDataByte(0x00);
    LCD_WriteCommand(ILI9341_CMD_GAMMASET); LCD_WriteDataByte(0x01);

    LCD_WriteCommand(ILI9341_CMD_SLPOUT);
    HAL_Delay(120);
    LCD_WriteCommand(ILI9341_CMD_DISPON);
    HAL_Delay(20);

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);
}

void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    /* static: avoids re-reserving 480 bytes on the stack on every call - this
     * project's stack is only 1KB, and DrawChar/DrawString call FillRect
     * through extra nested frames that a stack-local buffer here would risk
     * overflowing (worse for text than the shallower color-bar fill calls). */
    static uint8_t row_buf[ILI9341_WIDTH * 2];
    uint16_t row, i;

    if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
    {
        return;
    }
    if ((uint32_t)(x + w) > ILI9341_WIDTH)  { w = ILI9341_WIDTH - x; }
    if ((uint32_t)(y + h) > ILI9341_HEIGHT) { h = ILI9341_HEIGHT - y; }

    for (i = 0; i < w; i++)
    {
        row_buf[2 * i]     = (uint8_t)(color >> 8);
        row_buf[2 * i + 1] = (uint8_t)(color & 0xFF);
    }

    LCD_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
    LCD_CS_Low();
    for (row = 0; row < h; row++)
    {
        HAL_SPI_Transmit(ili9341_hspi, row_buf, (uint16_t)(w * 2), HAL_MAX_DELAY);
    }
    LCD_CS_High();
}

void ILI9341_FillScreen(uint16_t color)
{
    ILI9341_FillRect(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    ILI9341_FillRect(x, y, 1, 1, color);
}

void ILI9341_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color)
{
    ILI9341_FillRect(x, y, w, 1, color);
}

void ILI9341_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color)
{
    ILI9341_FillRect(x, y, 1, h, color);
}

void ILI9341_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    ILI9341_DrawHLine(x, y, w, color);
    ILI9341_DrawHLine(x, (uint16_t)(y + h - 1), w, color);
    ILI9341_DrawVLine(x, y, h, color);
    ILI9341_DrawVLine((uint16_t)(x + w - 1), y, h, color);
}

void ILI9341_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    /* Bresenham's line algorithm */
    int16_t dx = (int16_t)abs(x1 - x0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t dy = (int16_t)-abs(y1 - y0);
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx + dy;
    int16_t e2;

    for (;;)
    {
        ILI9341_DrawPixel((uint16_t)x0, (uint16_t)y0, color);
        if ((x0 == x1) && (y0 == y1))
        {
            break;
        }
        e2 = (int16_t)(2 * err);
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void ILI9341_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size)
{
    const uint8_t *glyph;
    uint8_t col, row;
    uint8_t index = (uint8_t)c;

    if ((index < GFX_FONT5X7_FIRST_CHAR) || (index > GFX_FONT5X7_LAST_CHAR))
    {
        index = GFX_FONT5X7_FIRST_CHAR; /* fall back to a blank glyph */
    }
    glyph = GFX_Font5x7[index - GFX_FONT5X7_FIRST_CHAR];

    for (col = 0; col < GFX_FONT5X7_WIDTH; col++)
    {
        uint8_t line = glyph[col];
        for (row = 0; row < GFX_FONT5X7_HEIGHT; row++)
        {
            uint16_t px = (uint16_t)(x + col * size);
            uint16_t py = (uint16_t)(y + row * size);
            uint16_t pixel_color = (line & (1U << row)) ? color : bg;
            ILI9341_FillRect(px, py, size, size, pixel_color);
        }
    }

    /* One blank column of spacing between characters */
    ILI9341_FillRect((uint16_t)(x + GFX_FONT5X7_WIDTH * size), y, size, (uint16_t)(GFX_FONT5X7_HEIGHT * size), bg);
}

void ILI9341_DrawString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size)
{
    while (*str != '\0')
    {
        ILI9341_DrawChar(x, y, *str++, color, bg, size);
        x = (uint16_t)(x + (GFX_FONT5X7_WIDTH + 1) * size);
    }
}

void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const unsigned short *img, uint16_t src_stride)
{
    /* static: same reasoning as ILI9341_FillRect - avoids reserving
     * 480 bytes on the stack, which this project's 1KB stack can't spare. */
    static uint8_t row_buf[ILI9341_WIDTH * 2];
    uint16_t row, col;
 
    if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
    {
        return;
    }
    if (src_stride == 0U) { src_stride = w; }
 
    if ((uint32_t)(x + w) > ILI9341_WIDTH)  { w = ILI9341_WIDTH - x; }
    if ((uint32_t)(y + h) > ILI9341_HEIGHT) { h = ILI9341_HEIGHT - y; }
 
    LCD_SetAddressWindow(x, y, x + w - 1, y + h - 1);
 
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
    LCD_CS_Low();
    for (row = 0; row < h; row++)
    {
        const unsigned short *src_row = &img[(uint32_t)row * src_stride];
 
        for (col = 0; col < w; col++)
        {
            unsigned short color = src_row[col];
            row_buf[2 * col]     = (uint8_t)(color >> 8);
            row_buf[2 * col + 1] = (uint8_t)(color & 0xFF);
        }
        HAL_SPI_Transmit(ili9341_hspi, row_buf, (uint16_t)(w * 2), HAL_MAX_DELAY);
    }
    LCD_CS_High();
}

/**
 * @brief  Draw a source image scaled (nearest-neighbor) to dst_w x dst_h.
 *
 * The ILI9341 has no hardware scaler - it only writes the bytes you send
 * to its GRAM window. This does the scaling on the MCU, one destination
 * row at a time, using fixed-point math to avoid a float divide per pixel.
 *
 * @param  img         Source image, row-major, src_w * src_h pixels.
 * @param  src_w/h     Source image dimensions.
 * @param  src_stride  Pixels per row in the source's original layout
 *                      (normally just src_w).
 * @param  dst_w/h     Desired on-screen size after scaling.
 */
// void ILI9341_DrawImageScaled(uint16_t x, uint16_t y, uint16_t src_w, uint16_t src_h, const unsigned short *img, uint16_t src_stride, uint16_t dst_w, uint16_t dst_h)
// {
//     static uint8_t row_buf[ILI9341_WIDTH * 2];
//     uint16_t row, col;
//     /* 16.16 fixed-point step size through the source image per dest pixel */
//     uint32_t x_step = ((uint32_t)src_w << 16) / dst_w;
//     uint32_t y_step = ((uint32_t)src_h << 16) / dst_h;
//     uint16_t w = dst_w, h = dst_h;

//     if (src_stride == 0U) { src_stride = src_w; }
//     if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
//     {
//         return;
//     }
//     if ((uint32_t)(x + w) > ILI9341_WIDTH)  { w = ILI9341_WIDTH - x; }
//     if ((uint32_t)(y + h) > ILI9341_HEIGHT) { h = ILI9341_HEIGHT - y; }

//     LCD_SetAddressWindow(x, y, x + w - 1, y + h - 1);

//     HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
//     LCD_CS_Low();
//     for (row = 0; row < h; row++)
//     {
//         uint16_t src_y = (uint16_t)(((uint32_t)row * y_step) >> 16);
//         const unsigned short *src_row = &img[(uint32_t)src_y * src_stride];

//         for (col = 0; col < w; col++)
//         {
//             uint16_t src_x = (uint16_t)(((uint32_t)col * x_step) >> 16);
//             unsigned short color = src_row[src_x];
//             row_buf[2 * col]     = (uint8_t)(color >> 8);
//             row_buf[2 * col + 1] = (uint8_t)(color & 0xFF);
//         }
//         HAL_SPI_Transmit(ili9341_hspi, row_buf, (uint16_t)(w * 2), HAL_MAX_DELAY);
//     }
//     LCD_CS_High();
// }

/**
 * @brief  Draw a source image scaled to dst_w x dst_h and rotated by
 *         `degrees` (clockwise, 0-360) about the center of the destination box.
 *
 * Uses inverse mapping: for every destination pixel, rotate backwards by
 * -degrees to find the corresponding (possibly fractional) source pixel,
 * then nearest-neighbor sample it. Destination pixels that land outside
 * the source image after the inverse rotation (the corners that rotate
 * "into" empty space) are filled black.
 *
 * sinf/cosf are computed once per call, not per pixel, but the per-pixel
 * float multiply-adds still cost more than the unrotated version - on an
 * MCU without an FPU this will be noticeably slower.
 */
void ILI9341_DrawImageScaled(uint16_t x, uint16_t y, uint16_t src_w, uint16_t src_h,
                              const unsigned short *img, uint16_t src_stride,
                              uint16_t dst_w, uint16_t dst_h, float degrees, uint16_t color)
{
    static uint8_t row_buf[ILI9341_WIDTH * 2];
    uint16_t row, col;
    uint16_t w = dst_w, h = dst_h;

    float theta = degrees * 3.14159265358979323846f / 180.0f;
    float cos_t = cosf(theta);
    float sin_t = sinf(theta);
    /* scale from "un-rotated destination offset" units into source pixels */
    float sx = (float)src_w / (float)dst_w;
    float sy = (float)src_h / (float)dst_h;
    float dst_cx = (float)dst_w / 2.0f;
    float dst_cy = (float)dst_h / 2.0f;
    float src_cx = (float)src_w / 2.0f;
    float src_cy = (float)src_h / 2.0f;

    if (src_stride == 0U) { src_stride = src_w; }
    if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
    {
        return;
    }
    if ((uint32_t)(x + w) > ILI9341_WIDTH)  { w = ILI9341_WIDTH - x; }
    if ((uint32_t)(y + h) > ILI9341_HEIGHT) { h = ILI9341_HEIGHT - y; }

    LCD_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
    LCD_CS_Low();
    for (row = 0; row < h; row++)
    {
        float oy = (float)row - dst_cy;

        for (col = 0; col < w; col++)
        {
            float ox = (float)col - dst_cx;
            /* inverse-rotate (note the sign) the dest offset back to
             * "un-rotated" space before mapping into the source image */
            float rx = ox * cos_t + oy * sin_t;
            float ry = -ox * sin_t + oy * cos_t;
            int32_t src_x = (int32_t)(rx * sx + src_cx);
            int32_t src_y = (int32_t)(ry * sy + src_cy);
            unsigned short color;

            if ((src_x < 0) || (src_x >= (int32_t)src_w) ||
                (src_y < 0) || (src_y >= (int32_t)src_h))
            {
                color = color; /* outside source image after rotation */
            }
            else
            {
                color = img[(uint32_t)src_y * src_stride + (uint32_t)src_x];
            }
            row_buf[2 * col]     = (uint8_t)(color >> 8);
            row_buf[2 * col + 1] = (uint8_t)(color & 0xFF);
        }
        HAL_SPI_Transmit(ili9341_hspi, row_buf, (uint16_t)(w * 2), HAL_MAX_DELAY);
    }
    LCD_CS_High();
}
