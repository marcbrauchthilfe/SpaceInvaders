#include "demos/joystick.h"
#include "hal/controls/joystick.h"
#include "pico/stdlib.h"
#include <stdio.h>

void joystick_demo_execute(void)
{
    joystick_init_simple_center();
    joystick_event_t event;

    while (true)
    {
        joystick_read(&event);
        printf("Joystick X: %.3f, Y: %.3f, Button: %s\n",
               event.x_norm,
               event.y_norm,
               event.button_pressed ? "Pressed" : "Released");
        sleep_ms(200);
    }
}