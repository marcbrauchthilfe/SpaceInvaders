#include <stdio.h>
#include "pico/stdlib.h"
#include "demos/display.h"
#include "demos/joystick.h"
#include "demos/leds.h"
#include "demos/dht11.h"
#include "demos/i2c_scan.h"
#include "demos/motion.h"
#include "demos/Abgabe_09.h"
#include "game/game.h"

int main()
{
    stdio_init_all();
    game_init();

    abgabe_09_execute();

    // joystick_demo_execute();
    // leds_demo_execute();
    // dht11_demo_execute();
    // i2c_scan_demo_execute();
    // motion_demo_execute();
    // distance_demo_execute();

    return 0;
}