#include "demos/display.h"
#include "hal/displays/st7735.h"
#include "pico/stdlib.h"
#include <stdio.h>

void init_display(void)
{
    // backlight pin
    gpio_init(2);
    gpio_set_dir(2, GPIO_OUT);
    gpio_put(2, 1);

    // 40 MHz SPI clock
    spi_init(spi0, 40 * 1000 * 1000);

    // Set SPI pin functions
    gpio_set_function(16, GPIO_FUNC_SPI);
    gpio_set_function(18, GPIO_FUNC_SPI);
    gpio_set_function(19, GPIO_FUNC_SPI);

    // Initialize ST7735 display
    st7735_init(spi0, 6, 17, 3, 0, false);
}

void display_demo_execute(void)
{

    init_display();
    st7735_begin();

    // black background
    st7735_fill_screen(st7735_rgb(0, 0, 0));
    // draw red rectangle
    st7735_fill_rect(10, 10, 50, 30, st7735_rgb(255, 0, 0));
    // draw green rectangle
    st7735_fill_rect(70, 10, 50, 30, st7735_rgb(0, 255, 0));
    // draw orange rectangle
    st7735_fill_rect(10, 50, 50, 30, st7735_rgb(255, 165, 0));
    // draw blue rectangle
    st7735_fill_rect(70, 50, 50, 30, st7735_rgb(0, 0, 255));
    // draw text
    st7735_draw_string(10, 100, "Hello display", st7735_rgb(255, 255, 255), st7735_rgb(0, 0, 0));

    while (true)
    {
        printf("Do nothing!\n");
        sleep_ms(1000);
    }
}

void display_draw_menu(void)
{
    init_display();
    st7735_begin();

    // black background
    st7735_fill_screen(st7735_rgb(0, 0, 0));
    // draw menu options
    st7735_draw_string(10, 20, "< Start Game", st7735_rgb(255, 255, 255), st7735_rgb(0, 0, 0));
    st7735_draw_string(10, 40, "> View Controls", st7735_rgb(255, 255, 255), st7735_rgb(0, 0, 0));
}

void display_draw_game_over(void)
{
    init_display();
    st7735_begin();

    // black background
    st7735_fill_screen(st7735_rgb(0, 0, 0));
    // draw game over message
    st7735_draw_string(20, 60, "Game Over!", st7735_rgb(255, 0, 0), st7735_rgb(0, 0, 0));
}

void display_draw_controls(void)
{
    init_display();
    st7735_begin();

    // black background
    st7735_fill_screen(st7735_rgb(0, 0, 0));
    // draw controls information
    st7735_draw_string(10, 20, "Controls:", st7735_rgb(255, 255, 255), st7735_rgb(0, 0, 0));
    st7735_draw_string(10, 40, "Top Button: Shoot", st7735_rgb(255, 255, 255), st7735_rgb(0, 0, 0));
    st7735_draw_string(10, 60, "Joystick: Move left or right", st7735_rgb(255, 255, 255), st7735_rgb(0, 0, 0));
}

void display_draw_playing(void)
{
    init_display();
    st7735_begin();

    // black background
    st7735_fill_screen(st7735_rgb(0, 0, 0));
    
    // draw playing state (placeholder)
    st7735_draw_string(10, 20, "Game in Progress...", st7735_rgb(255, 255, 255), st7735_rgb(0, 0, 0));
}