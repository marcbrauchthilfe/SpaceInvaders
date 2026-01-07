// based on https://github.com/PDBeal/pico-ws2812

#ifndef _WS2812_C_H_
#define _WS2812_C_H_

#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "ws2812.pio.h"

#include <stdlib.h>
#include <string.h>

typedef struct
{
    uint16_t numLEDs;  // Count of pixels
    uint8_t *pixels;   // Buffer for pixel data (3 bytes per pixel: R, G, B)
    uint8_t pixelGpio; // GPIO Pin
    PIO pixelPio;      // PIO instance (pio0 or pio1)
    int pixelSm;       // PIO state machine
} WS2812;

void ws2812_init(WS2812 *ws, uint16_t num, uint8_t pin, PIO pio, int sm);
void ws2812_init_auto_sm(WS2812 *ws, uint16_t num, uint8_t pin);
void ws2812_deinit(WS2812 *ws);
void ws2812_begin(WS2812 *ws);
void ws2812_show(WS2812 *ws);
void ws2812_set_pixel_color_rgb(WS2812 *ws, uint16_t led, uint8_t red, uint8_t green, uint8_t blue);
void ws2812_clear(WS2812 *ws);
uint16_t ws2812_num_pixels(WS2812 *ws);
uint32_t ws2812_get_pixel_color(WS2812 *ws, uint16_t led);

#endif // _WS2812_C_H_