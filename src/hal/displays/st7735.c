// based on https://joy-it.net/files/files/Produkte/RB-P-XPLR/RB-P-XPLR_Examples-and-libraries.zip

#include "hal/displays/st7735.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// ST7735 commands
#define DELAY 0x80

#define ST7735_NOP 0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID 0x04
#define ST7735_RDDST 0x09

#define ST7735_SLPIN 0x10
#define ST7735_SLPOUT 0x11
#define ST7735_PTLON 0x12
#define ST7735_NORON 0x13

#define ST7735_INVOFF 0x20
#define ST7735_INVON 0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON 0x29
#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_RAMWR 0x2C
#define ST7735_RAMRD 0x2E

#define ST7735_PTLAR 0x30
#define ST7735_COLMOD 0x3A
#define ST7735_MADCTL 0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// MADCTL rotations
#define ST7735_MADCTL_MY 0x80
#define ST7735_MADCTL_MX 0x40
#define ST7735_MADCTL_MV 0x20
#define ST7735_MADCTL_ML 0x10
#define ST7735_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

// Internal state (static)
static spi_inst_t *_spi;
static uint _rst_pin;
static uint _ce_pin;
static uint _dc_pin;
static uint _offset;
static int _width;
static int _height;
static uint8_t _color_mode;

// Font 5x7 pixels
static const uint8_t font5x7[480] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x07,
    0x00, 0x07, 0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14, 0x24, 0x2a, 0x7f, 0x2a,
    0x12, 0x23, 0x13, 0x08, 0x64, 0x62, 0x36, 0x49, 0x55, 0x22, 0x50, 0x00,
    0x05, 0x03, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x41, 0x00, 0x00, 0x41, 0x22,
    0x1c, 0x00, 0x08, 0x2a, 0x1c, 0x2a, 0x08, 0x08, 0x08, 0x3e, 0x08, 0x08,
    0x00, 0x50, 0x30, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x60,
    0x60, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x3e, 0x51, 0x49, 0x45,
    0x3e, 0x00, 0x42, 0x7f, 0x40, 0x00, 0x42, 0x61, 0x51, 0x49, 0x46, 0x21,
    0x41, 0x45, 0x4b, 0x31, 0x18, 0x14, 0x12, 0x7f, 0x10, 0x27, 0x45, 0x45,
    0x45, 0x39, 0x3c, 0x4a, 0x49, 0x49, 0x30, 0x01, 0x71, 0x09, 0x05, 0x03,
    0x36, 0x49, 0x49, 0x49, 0x36, 0x06, 0x49, 0x49, 0x29, 0x1e, 0x00, 0x36,
    0x36, 0x00, 0x00, 0x00, 0x56, 0x36, 0x00, 0x00, 0x00, 0x08, 0x14, 0x22,
    0x41, 0x14, 0x14, 0x14, 0x14, 0x14, 0x41, 0x22, 0x14, 0x08, 0x00, 0x02,
    0x01, 0x51, 0x09, 0x06, 0x32, 0x49, 0x79, 0x41, 0x3e, 0x7e, 0x11, 0x11,
    0x11, 0x7e, 0x7f, 0x49, 0x49, 0x49, 0x36, 0x3e, 0x41, 0x41, 0x41, 0x22,
    0x7f, 0x41, 0x41, 0x22, 0x1c, 0x7f, 0x49, 0x49, 0x49, 0x41, 0x7f, 0x09,
    0x09, 0x01, 0x01, 0x3e, 0x41, 0x41, 0x51, 0x32, 0x7f, 0x08, 0x08, 0x08,
    0x7f, 0x00, 0x41, 0x7f, 0x41, 0x00, 0x20, 0x40, 0x41, 0x3f, 0x01, 0x7f,
    0x08, 0x14, 0x22, 0x41, 0x7f, 0x40, 0x40, 0x40, 0x40, 0x7f, 0x02, 0x04,
    0x02, 0x7f, 0x7f, 0x04, 0x08, 0x10, 0x7f, 0x3e, 0x41, 0x41, 0x41, 0x3e,
    0x7f, 0x09, 0x09, 0x09, 0x06, 0x3e, 0x41, 0x51, 0x21, 0x5e, 0x7f, 0x09,
    0x19, 0x29, 0x46, 0x46, 0x49, 0x49, 0x49, 0x31, 0x01, 0x01, 0x7f, 0x01,
    0x01, 0x3f, 0x40, 0x40, 0x40, 0x3f, 0x1f, 0x20, 0x40, 0x20, 0x1f, 0x7f,
    0x20, 0x18, 0x20, 0x7f, 0x63, 0x14, 0x08, 0x14, 0x63, 0x03, 0x04, 0x78,
    0x04, 0x03, 0x61, 0x51, 0x49, 0x45, 0x43, 0x00, 0x00, 0x7f, 0x41, 0x41,
    0x02, 0x04, 0x08, 0x10, 0x20, 0x41, 0x41, 0x7f, 0x00, 0x00, 0x04, 0x02,
    0x01, 0x02, 0x04, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x01, 0x02, 0x04,
    0x00, 0x20, 0x54, 0x54, 0x54, 0x78, 0x7f, 0x48, 0x44, 0x44, 0x38, 0x38,
    0x44, 0x44, 0x44, 0x20, 0x38, 0x44, 0x44, 0x48, 0x7f, 0x38, 0x54, 0x54,
    0x54, 0x18, 0x08, 0x7e, 0x09, 0x01, 0x02, 0x08, 0x14, 0x54, 0x54, 0x3c,
    0x7f, 0x08, 0x04, 0x04, 0x78, 0x00, 0x44, 0x7d, 0x40, 0x00, 0x20, 0x40,
    0x44, 0x3d, 0x00, 0x00, 0x7f, 0x10, 0x28, 0x44, 0x00, 0x41, 0x7f, 0x40,
    0x00, 0x7c, 0x04, 0x18, 0x04, 0x78, 0x7c, 0x08, 0x04, 0x04, 0x78, 0x38,
    0x44, 0x44, 0x44, 0x38, 0x7c, 0x14, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
    0x18, 0x7c, 0x7c, 0x08, 0x04, 0x04, 0x08, 0x48, 0x54, 0x54, 0x54, 0x20,
    0x04, 0x3f, 0x44, 0x40, 0x20, 0x3c, 0x40, 0x40, 0x20, 0x7c, 0x1c, 0x20,
    0x40, 0x20, 0x1c, 0x3c, 0x40, 0x30, 0x40, 0x3c, 0x44, 0x28, 0x10, 0x28,
    0x44, 0x0c, 0x50, 0x50, 0x50, 0x3c, 0x44, 0x64, 0x54, 0x4c, 0x44, 0x00,
    0x08, 0x36, 0x41, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x41, 0x36,
    0x08, 0x00, 0x08, 0x08, 0x2a, 0x1c, 0x08, 0x08, 0x1c, 0x2a, 0x08, 0x08};

// Sends a command (DC=low) to the display.
static void st7735_write_cmd(uint8_t cmd)
{
    gpio_put(_dc_pin, 0);
    gpio_put(_ce_pin, 0);
    spi_write_blocking(_spi, &cmd, 1);
    gpio_put(_ce_pin, 1);
}

// Sends data (DC=high) to the display.
static void st7735_write_data(uint8_t data)
{
    gpio_put(_dc_pin, 1);
    gpio_put(_ce_pin, 0);
    spi_write_blocking(_spi, &data, 1);
    gpio_put(_ce_pin, 1);
}

// Sends a buffer of data (DC=high).
void st7735_write_data_buffer(const uint8_t *buffer, size_t len)
{
    gpio_put(_dc_pin, 1);
    gpio_put(_ce_pin, 0);
    spi_write_blocking(_spi, buffer, len);
    gpio_put(_ce_pin, 1);
}

// Sets the address window for pixel operations.
void st7735_set_addr_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    st7735_write_cmd(ST7735_CASET); // Column addr set
    st7735_write_data(0x00);
    st7735_write_data(x0 + _offset); // XSTART
    st7735_write_data(0x00);
    st7735_write_data(x1 + _offset); // XEND

    st7735_write_cmd(ST7735_RASET); // Row addr set
    st7735_write_data(0x00);
    st7735_write_data(y0 + _offset); // YSTART
    st7735_write_data(0x00);
    st7735_write_data(y1 + _offset); // YEND

    st7735_write_cmd(ST7735_RAMWR); // write to RAM
}

void st7735_init(spi_inst_t *spi, uint rst_pin, uint ce_pin, uint dc_pin, uint offset, bool is_bgr)
{
    _spi = spi;
    _rst_pin = rst_pin;
    _ce_pin = ce_pin;
    _dc_pin = dc_pin;
    _offset = offset;

    _width = ST7735_TFTWIDTH;
    _height = ST7735_TFTHEIGHT;

    if (is_bgr)
    {
        _color_mode = ST7735_MADCTL_BGR;
    }
    else
    {
        _color_mode = ST7735_MADCTL_RGB;
    }

    // GPIO initialization
    gpio_init(_rst_pin);
    gpio_init(_ce_pin);
    gpio_init(_dc_pin);

    gpio_set_dir(_rst_pin, GPIO_OUT);
    gpio_set_dir(_ce_pin, GPIO_OUT);
    gpio_set_dir(_dc_pin, GPIO_OUT);

    gpio_put(_ce_pin, 1);
    gpio_put(_dc_pin, 1);
}

void st7735_reset()
{
    gpio_put(_rst_pin, 0);
    sleep_ms(50);
    gpio_put(_rst_pin, 1);
    sleep_ms(50);
}

void st7735_begin()
{
    st7735_reset();

    // Initialization sequence (translated from Python code)
    static const uint8_t init_cmds[] = {
        ST7735_SWRESET, DELAY, //  1: Software reset
        150,
        ST7735_SLPOUT, DELAY, //  2: Out of sleep mode
        255,
        ST7735_FRMCTR1, 3, //  3: Frame rate ctrl - normal mode
        0x01, 0x2C, 0x2D,
        ST7735_FRMCTR2, 3, //  4: Frame rate control - idle mode
        0x01, 0x2C, 0x2D,
        ST7735_FRMCTR3, 6, //  5: Frame rate ctrl - partial mode
        0x01, 0x2C, 0x2D,
        0x01, 0x2C, 0x2D,
        ST7735_INVCTR, 1, //  6: Display inversion ctrl
        0x07,
        ST7735_PWCTR1, 3, //  7: Power control
        0xA2,
        0x02,
        0x84,
        ST7735_PWCTR2, 1, //  8: Power control
        0xC5,
        ST7735_PWCTR3, 2, //  9: Power control
        0x0A,
        0x00,
        ST7735_PWCTR4, 2, // 10: Power control
        0x8A,
        0x2A,
        ST7735_PWCTR5, 2, // 11: Power control
        0x8A, 0xEE,
        ST7735_VMCTR1, 1, // 12: Power control
        0x0E,
        ST7735_INVOFF, 0, // 13: Don't invert display
        ST7735_MADCTL, 1, // 14: Memory access control
        0xC8,
        ST7735_COLMOD, 1, // 15: set color mode
        0x05,             //     16-bit color
        ST7735_CASET, 4,  //  1: Column addr set
        0x00, 0x00,
        0x00, 0x7F,
        ST7735_RASET, 4, //  2: Row addr set
        0x00, 0x00,
        0x00, 0x9F,
        ST7735_GMCTRP1, 16, //  1: Gamma positive
        0x02, 0x1c, 0x07, 0x12,
        0x37, 0x32, 0x29, 0x2d,
        0x29, 0x25, 0x2B, 0x39,
        0x00, 0x01, 0x03, 0x10,
        ST7735_GMCTRN1, 16, //  2: Gamma negative
        0x03, 0x1d, 0x07, 0x06,
        0x2E, 0x2C, 0x29, 0x2D,
        0x2E, 0x2E, 0x37, 0x3F,
        0x00, 0x00, 0x02, 0x10,
        ST7735_NORON, DELAY, //  3: Normal display on
        10,
        ST7735_DISPON, DELAY, //  4: Main screen turn on
        100};

    // Process the command list (like in the Python code)
    int argcount = 0;
    int cmd_state = 1; // 1 = expecting command, 0 = expecting arguments/delay
    int delay = 0;

    for (size_t i = 0; i < sizeof(init_cmds); i++)
    {
        uint8_t c = init_cmds[i];
        if (argcount == 0)
        {
            if (delay)
            {
                uint16_t delay_ms = c;
                if (delay_ms == 255)
                    delay_ms = 500; // Special case from Python code
                sleep_ms(delay_ms);
                delay = 0;
            }
            else
            {
                if (cmd_state == 1)
                {
                    st7735_write_cmd(c);
                    cmd_state = 0;
                }
                else
                {
                    argcount = c & ~DELAY;
                    delay = c & DELAY;
                    cmd_state = 1;
                }
            }
        }
        else
        {
            st7735_write_data(c);
            argcount--;
        }
    }

    // Last command from the Python code (set color filter)
    st7735_write_cmd(ST7735_MADCTL);
    st7735_write_data(0xC0 | _color_mode);
}

void st7735_draw_pixel(int x, int y, uint16_t color)
{
    if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
    {
        return;
    }

    st7735_set_addr_window(x, y, x, y);
    uint8_t data[2] = {color >> 8, color & 0xFF};
    st7735_write_data_buffer(data, 2);
}

void st7735_fill_rect(int x, int y, int w, int h, uint16_t color)
{
    if ((x >= _width) || (y >= _height))
    {
        return;
    }
    if ((x + w - 1) >= _width)
    {
        w = _width - x;
    }
    if ((y + h - 1) >= _height)
    {
        h = _height - y;
    }

    st7735_set_addr_window(x, y, x + w - 1, y + h - 1);

    // Create buffer for ONE line
    size_t line_buffer_size = w * 2;
    uint8_t *line_buffer = (uint8_t *)malloc(line_buffer_size);
    if (!line_buffer)
    {
        // Memory error, possibly abort
        return;
    }

    // Fill the line buffer
    for (int i = 0; i < w; i++)
    {
        line_buffer[i * 2] = color >> 8;       // High-Byte
        line_buffer[i * 2 + 1] = color & 0xFF; // Low-Byte
    }

    // Send the line buffer h times
    gpio_put(_dc_pin, 1);
    gpio_put(_ce_pin, 0);
    for (int j = 0; j < h; j++)
    {
        spi_write_blocking(_spi, line_buffer, line_buffer_size);
    }
    gpio_put(_ce_pin, 1);

    free(line_buffer);
}

void st7735_fill_screen(uint16_t color)
{
    st7735_fill_rect(0, 0, _width, _height, color);
}

void st7735_draw_buffer(int x, int y, int w, int h, const uint8_t *buffer)
{
    printf("Draw BMP at (%d,%d) size %dx%d\n", x, y, w, h);
    if ((x >= _width) || (y >= _height))
    {
        return;
    }
    if ((x + w - 1) >= _width)
    {
        w = _width - x;
    }
    if ((y + h - 1) >= _height)
    {
        h = _height - y;
    }

    st7735_set_addr_window(x, y, x + w - 1, y + h - 1);
    st7735_write_data_buffer(buffer, w * h * 2);
}

void st7735_draw_char(int x, int y, char ch, uint16_t color, uint16_t bg_color)
{
    // Valid range of the font (ASCII 0x20 to 0x7F)
    if (ch < 0x20 || ch > 0x7F)
    {
        ch = 0x20; // Show space for invalid characters
    }

    // 6x8 pixel buffer (6 columns, 8 rows, 2 bytes/pixel)
    uint8_t char_image[6 * 8 * 2];

    // Pointer to the 5 bytes of the character in the font
    const uint8_t *glyph = &font5x7[(ch - 0x20) * 5];

    // Add the 6th column (space)
    uint8_t glyph_with_space[6];
    memcpy(glyph_with_space, glyph, 5);
    glyph_with_space[5] = 0x00; // Last column is always empty

    int buf_idx = 0;
    for (int j = 0; j < 8; j++)
    { // Iterate over bits/rows (0-7)
        for (int i = 0; i < 6; i++)
        { // Iterate over bytes/columns (0-5)
            if ((glyph_with_space[i] >> j) & 1)
            {
                // Foreground
                // CORRECTION 3A
                char_image[buf_idx++] = color >> 8;   // High-Byte
                char_image[buf_idx++] = color & 0xFF; // Low-Byte
            }
            else
            {
                // Background
                // CORRECTION 3B
                char_image[buf_idx++] = bg_color >> 8;   // High-Byte
                char_image[buf_idx++] = bg_color & 0xFF; // Low-Byte
            }
        }
    }

    st7735_draw_buffer(x, y, 6, 8, char_image);
}

void st7735_draw_string(int x, int y, const char *str, uint16_t color, uint16_t bg_color)
{
    while (*str)
    {
        st7735_draw_char(x, y, *str, color, bg_color);
        x += 6; // Width of each character
        str++;
    }
}

void st7735_set_rotation(uint8_t m)
{
    st7735_write_cmd(ST7735_MADCTL);
    uint8_t rotation = m % 4; // 0, 1, 2, 3

    switch (rotation)
    {
    case 0: // 0 degrees
        st7735_write_data(ST7735_MADCTL_MX | ST7735_MADCTL_MY | _color_mode);
        _width = ST7735_TFTWIDTH;
        _height = ST7735_TFTHEIGHT;
        break;
    case 1: // 90 degrees
        st7735_write_data(ST7735_MADCTL_MY | ST7735_MADCTL_MV | _color_mode);
        _width = ST7735_TFTHEIGHT;
        _height = ST7735_TFTWIDTH;
        break;
    case 2: // 180 degrees
        st7735_write_data(_color_mode);
        _width = ST7735_TFTWIDTH;
        _height = ST7735_TFTHEIGHT;
        break;
    case 3: // 270 degrees
        st7735_write_data(ST7735_MADCTL_MX | ST7735_MADCTL_MV | _color_mode);
        _width = ST7735_TFTHEIGHT;
        _height = ST7735_TFTWIDTH;
        break;
    }
}

uint16_t st7735_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}