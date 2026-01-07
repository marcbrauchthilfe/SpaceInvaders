// based on https://joy-it.net/files/files/Produkte/RB-P-XPLR/RB-P-XPLR_Examples-and-libraries.zip
#ifndef ST7735_H
#define ST7735_H

#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdbool.h>

#define ST7735_TFTWIDTH 128
#define ST7735_TFTHEIGHT 160

void st7735_init(spi_inst_t *spi, uint rst_pin, uint ce_pin, uint dc_pin, uint offset, bool is_bgr);
void st7735_begin();
void st7735_reset();
void st7735_draw_pixel(int x, int y, uint16_t color);
void st7735_fill_rect(int x, int y, int w, int h, uint16_t color);
void st7735_fill_screen(uint16_t color);
void st7735_draw_string(int x, int y, const char *str, uint16_t color, uint16_t bg_color);
uint16_t st7735_rgb(uint8_t r, uint8_t g, uint8_t b);
void st7735_set_addr_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void st7735_write_data_buffer(const uint8_t *buffer, size_t len);

#endif // ST7735_H