#include "hal/controls/joystick.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/time.h"
#include <stdio.h>

// Pin-Definitions
const uint BTN_PIN = 22;
const uint ADC0_PIN = 26; // Joystick X
const uint ADC1_PIN = 27; // Joystick Y

// ADC Inputs
const uint ADC_INPUT_X = 0; // Corresponds to GPIO 26
const uint ADC_INPUT_Y = 1; // Corresponds to GPIO 27

// Settings
const uint16_t DEADZONE = 1500; // Deadzone in 16-bit scaled values

// Global calibration values
// These variables are set by one of the 'init' functions
// and used by 'joystick_read'.
static uint16_t g_cal_min_x;
static uint16_t g_cal_max_x;
static uint16_t g_cal_center_x;

static uint16_t g_cal_min_y;
static uint16_t g_cal_max_y;
static uint16_t g_cal_center_y;

// FIRST: Initialize the hardware (used by both methods)
void joystick_hardware_init(void)
{
    // Initialize button (GPIO 22)
    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_pull_up(BTN_PIN);

    // Initialize ADC
    adc_init();
    adc_gpio_init(ADC0_PIN); // Enable GPIO 26 for ADC
    adc_gpio_init(ADC1_PIN); // Enable GPIO 27 for ADC
}

// METHOD 1: Simple "Center" Calibration
void joystick_init_simple_center(void)
{
    joystick_hardware_init();
    printf("Simple calibration started... Do not touch the joystick!\n");

    const int CALIBRATION_SAMPLES = 100;
    uint32_t sum_x_12bit = 0;
    uint32_t sum_y_12bit = 0;

    for (int i = 0; i < CALIBRATION_SAMPLES; i++)
    {
        adc_select_input(ADC_INPUT_X);
        sum_x_12bit += adc_read();
        adc_select_input(ADC_INPUT_Y);
        sum_y_12bit += adc_read();
        sleep_ms(2);
    }

    // Calculate and scale center
    g_cal_center_x = (sum_x_12bit / CALIBRATION_SAMPLES) << 4;
    g_cal_center_y = (sum_y_12bit / CALIBRATION_SAMPLES) << 4;

    // Assume Min/Max values (theoretical maximum)
    g_cal_min_x = 0;
    g_cal_max_x = (4095 << 4); // 65520
    g_cal_min_y = 0;
    g_cal_max_y = (4095 << 4); // 65520

    printf("Simple calibration ended.\n");
    printf("Center X: %u, Center Y: %u\n", g_cal_center_x, g_cal_center_y);
}

// METHOD 2: Full "Min/Max/Center" Calibration
void joystick_init_full_range(void)
{
    joystick_hardware_init();
    printf("Full calibration started...\n");
    printf("Move the joystick to all corners for 5 seconds!\n");

    // Initial values for Min/Max (12-bit)
    uint16_t current_min_x = 4095;
    uint16_t current_max_x = 0;
    uint16_t current_min_y = 4095;
    uint16_t current_max_y = 0;

    absolute_time_t end_time = make_timeout_time_ms(5000); // 5 seconds

    while (!time_reached(end_time))
    {
        adc_select_input(ADC_INPUT_X);
        uint16_t x_12bit = adc_read();
        adc_select_input(ADC_INPUT_Y);
        uint16_t y_12bit = adc_read();

        if (x_12bit < current_min_x)
            current_min_x = x_12bit;
        if (x_12bit > current_max_x)
            current_max_x = x_12bit;

        if (y_12bit < current_min_y)
            current_min_y = y_12bit;
        if (y_12bit > current_max_y)
            current_max_y = y_12bit;

        sleep_ms(1);
    }

    // Scale values to 16-bit and store globally
    g_cal_min_x = current_min_x << 4;
    g_cal_max_x = current_max_x << 4;
    g_cal_min_y = current_min_y << 4;
    g_cal_max_y = current_max_y << 4;

    // The "center" is the average of the found Min/Max values
    g_cal_center_x = (g_cal_min_x + g_cal_max_x) / 2;
    g_cal_center_y = (g_cal_min_y + g_cal_max_y) / 2;

    printf("Full calibration ended.\n");
    printf("X-axis: Min=%u, Max=%u, Center=%u\n", g_cal_min_x, g_cal_max_x, g_cal_center_x);
    printf("Y-axis: Min=%u, Max=%u, Center=%u\n", g_cal_min_y, g_cal_max_y, g_cal_center_y);
}

void joystick_read(joystick_event_t *event)
{

    // Ensure we have a valid pointer
    if (event == NULL)
    {
        return;
    }

    // Read raw values and scale to 16-bit
    adc_select_input(ADC_INPUT_X);
    uint16_t xr = adc_read() << 4;

    adc_select_input(ADC_INPUT_Y);
    uint16_t yr = adc_read() << 4;

    // Calculate relative position to calibrated center
    int32_t x_relativ = (int32_t)xr - (int32_t)g_cal_center_x;
    int32_t y_relativ = (int32_t)yr - (int32_t)g_cal_center_y;

    float x_norm = 0.0f;
    float y_norm = 0.0f;

    // X-axis normalization
    if (x_relativ > DEADZONE)
    {
        // Positive range: from center to max
        int32_t pos_range_x = (int32_t)g_cal_max_x - (int32_t)g_cal_center_x;
        // Prevent division by zero in case calibration fails
        if (pos_range_x > 0)
        {
            x_norm = (float)x_relativ / (float)pos_range_x;
        }
    }
    else if (x_relativ < -DEADZONE)
    {
        // Negative range: from center to min
        int32_t neg_range_x = (int32_t)g_cal_center_x - (int32_t)g_cal_min_x;
        if (neg_range_x > 0)
        {
            x_norm = (float)x_relativ / (float)neg_range_x;
        }
    }
    // In between (in deadzone) x_norm remains 0.0f
    // Y-axis normalization
    if (y_relativ > DEADZONE)
    {
        int32_t pos_range_y = (int32_t)g_cal_max_y - (int32_t)g_cal_center_y;
        if (pos_range_y > 0)
        {
            y_norm = (float)y_relativ / (float)pos_range_y;
        }
    }
    else if (y_relativ < -DEADZONE)
    {
        int32_t neg_range_y = (int32_t)g_cal_center_y - (int32_t)g_cal_min_y;
        if (neg_range_y > 0)
        {
            y_norm = (float)y_relativ / (float)neg_range_y;
        }
    }

    // Clipping, in case the read value is outside
    // the calibrated Min/Max range
    if (x_norm > 1.0f)
        x_norm = 1.0f;
    if (x_norm < -1.0f)
        x_norm = -1.0f;
    if (y_norm > 1.0f)
        y_norm = 1.0f;
    if (y_norm < -1.0f)
        y_norm = -1.0f;

    bool btn_pressed = (gpio_get(BTN_PIN) == 0);

    event->x_norm = x_norm;
    event->y_norm = y_norm;
    event->button_pressed = btn_pressed;
}
