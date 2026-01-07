#include "demos/i2c_scan.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

bool static reserved_addr(uint8_t addr)
{
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void i2c_scan_demo_execute(void)
{
    // I2C initialization with 100 kHz
    i2c_init(I2C_PORT, 100 * 1000);

    // Assign GPIO functions
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    // Enable pull-ups (important if no external ones are installed)
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Binary Info for picotool (optional, but "good practice")
    bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));

    printf("\n   0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

    for (int addr = 0; addr < (1 << 7); ++addr)
    {
        if (addr % 16 == 0)
        {
            printf("%02x ", addr);
        }

        // Skip reserved addresses
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            // Try to read 1 byte
            // If ret > 0, the device responded (ACK)
            // If ret < 0, no one responded (NAK)
            ret = i2c_read_blocking(I2C_PORT, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }

    printf("Scan finished.\n");

    // Infinite loop to prevent the program from ending
    while (1)
    {
        tight_loop_contents();
    }
}