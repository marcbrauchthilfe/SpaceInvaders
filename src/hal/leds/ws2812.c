// based on https://github.com/PDBeal/pico-ws2812
#include "hal/leds/ws2812.h"

static void ws2812_alloc(WS2812 *ws, uint16_t num)
{
    ws->numLEDs = ((ws->pixels = (uint8_t *)calloc(num, 3)) != NULL) ? num : 0;
}

void ws2812_init(WS2812 *ws, uint16_t num, uint8_t pin, PIO pio, int sm)
{
    ws2812_alloc(ws, num);
    ws->pixelSm = sm;
    ws->pixelPio = pio;
    ws->pixelGpio = pin;
}

void ws2812_init_auto_sm(WS2812 *ws, uint16_t num, uint8_t pin)
{
    ws2812_alloc(ws, num);

    PIO pio = pio0;
    int sm;
    // Find free state machine (pio0)
    sm = pio_claim_unused_sm(pio, false); // don't panic
    // Try pio1 if pio0 is full
    if (sm < 0)
    {
        pio = pio1;
        sm = pio_claim_unused_sm(pio, true); // panic if no SM is free
    }

    ws->pixelSm = sm;
    ws->pixelPio = pio;
    ws->pixelGpio = pin;
}

void ws2812_deinit(WS2812 *ws)
{
    if (ws->pixels)
    {
        free(ws->pixels);
    }
    ws->pixels = NULL;
    ws->numLEDs = 0;
}

void ws2812_begin(WS2812 *ws)
{
    // Load the PIO program into the instruction memory
    uint offset = pio_add_program(ws->pixelPio, &ws2812_program);
    // Initialize the state machine with the program
    ws2812_program_init(ws->pixelPio, ws->pixelSm, offset, ws->pixelGpio, 800000, false);
}

void ws2812_set_pixel_color_rgb(WS2812 *ws, uint16_t led, uint8_t red, uint8_t green, uint8_t blue)
{
    if (led < ws->numLEDs)
    {
        uint8_t *p = &ws->pixels[led * 3];
        *p++ = green; // G
        *p++ = red;   // R
        *p++ = blue;  // B
    }
}

void ws2812_set_pixel_color_packed(WS2812 *ws, uint16_t led, uint32_t color)
{
    if (led < ws->numLEDs)
    {
        uint8_t *p = &ws->pixels[led * 3];
        *p++ = (uint8_t)(color >> 16); // Red
        *p++ = (uint8_t)(color >> 8);  // Green
        *p++ = (uint8_t)color;         // Blue
    }
}

void ws2812_clear(WS2812 *ws)
{
    if (ws->pixels != NULL)
    {
        // Clear the entire buffer to 0
        memset(ws->pixels, 0, ws->numLEDs * 3);
    }
}

void ws2812_show(WS2812 *ws)
{
    for (uint16_t i = 0; i < ws->numLEDs; i++)
    {
        uint8_t redPtr = ws->pixels[i * 3];
        uint8_t greenPtr = ws->pixels[(i * 3) + 1];
        uint8_t bluePtr = ws->pixels[(i * 3) + 2];

        // The WS2812 PIO expects data in the format 0x00GGRRBB
        // (because the PIO program sends in GRB order)
        uint32_t colorData = ((uint32_t)(greenPtr) << 16) |
                             ((uint32_t)(redPtr) << 8) |
                             (uint32_t)(bluePtr);

        // Send the data to the PIO FIFO.
        // The PIO program is designed to expect 24 bits.
        // We shift the 24 bits (0xGGRRBB) 8 positions to the left,
        // so they are at the MSBs of the 32-bit FIFO (0xGGRRBB00).
        pio_sm_put_blocking(ws->pixelPio, ws->pixelSm, colorData << 8u);
    }
}

void ws2812_fill_pixel_color(WS2812 *ws, uint8_t red, uint8_t green, uint8_t blue)
{
    for (uint16_t i = 0; i < ws->numLEDs; i++)
    {
        uint8_t *p = &ws->pixels[i * 3];
        *p++ = red;
        *p++ = green;
        *p++ = blue;
    }
}

void ws2812_update_length(WS2812 *ws, uint16_t num)
{
    if (ws->pixels != NULL)
    {
        free(ws->pixels); // Free old memory
    }
    // Allocate new memory (initialized to 0 by alloc)
    ws2812_alloc(ws, num);
}

uint16_t ws2812_num_pixels(WS2812 *ws)
{
    return ws->numLEDs;
}

uint32_t ws2812_get_pixel_color(WS2812 *ws, uint16_t led)
{
    if (led < ws->numLEDs)
    {
        uint16_t ofs = led * 3;
        // Pack the R,G,B data from the buffer into a 0x00RRGGBB format
        return ((uint32_t)ws->pixels[ofs] << 16) |    // R
               ((uint16_t)ws->pixels[ofs + 1] << 8) | // G
               ((uint16_t)ws->pixels[ofs + 2]);       // B
    }

    return 0; // Out of range
}