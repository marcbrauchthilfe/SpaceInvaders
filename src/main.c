#include <stdio.h>
#include "pico/stdlib.h"
#include "include/demos/Abgabe_09.h"
#include "game/game.h"
#include "game/handling.h"

int main(void)
{
    stdio_init_all();
    sleep_ms(2000); // Zeit für USB-Serial

    game_init();       // Display + Startmenü
    handling_execute(); // Game-Loop (läuft endlos)

    // Wird nie erreicht
    while (true) {  
        tight_loop_contents();
    }
}


    // Weitere Demos zum Ausprobieren:
    // joystick_demo_execute();
    // leds_demo_execute();
    // dht11_demo_execute();
    // i2c_scan_demo_execute();
    // motion_demo_execute();
    // distance_demo_execute();