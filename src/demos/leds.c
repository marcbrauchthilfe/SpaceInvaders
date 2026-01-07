#include "demos/leds.h"
#include "hal/leds/ws2812.h"
#include "pico/stdlib.h"
#include <stdio.h>

void leds_demo_execute(void)
{
    WS2812 ws;
    const uint16_t num_leds = 4;
    const uint8_t pin = 1; // GPIO Pin for the LED strip

    ws2812_init_auto_sm(&ws, num_leds, pin);
    ws2812_begin(&ws);

    // Simple demo effect: running light
    while (true)
    {
        for (uint16_t i = 0; i < num_leds; i++)
        {
            ws2812_clear(&ws);
            ws2812_set_pixel_color_rgb(&ws, i, 255, 0, 0); // Red
            ws2812_show(&ws);
            sleep_ms(200);
        }
    }
    ws2812_deinit(&ws);
}