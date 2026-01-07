#include "include/demos/Abgabe_09.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

#define BUTTON_PIN 15

void abgabe_09_execute(void)
{
    // Initialisiere den GPIO-Pin
    gpio_init(BUTTON_PIN);

    // Setze den Pin als Eingang (Input)
    gpio_set_dir(BUTTON_PIN, GPIO_IN);

    // Aktiviere den internen Pull-Up-Widerstand.
    // Der Pin ist standardmäßig HIGH (1).
    // Er wird LOW (0), wenn der Taster gedrückt wird (Verbindung mit GND).
    gpio_pull_up(BUTTON_PIN);

    printf("Read button state (GPIO %d) started.\n", BUTTON_PIN);
    printf("Press the button (connected to GND).\n\n");

    // Endlosschleife
    while (true)
    {
        // Lese den Zustand des GPIO-Pins
        bool button_state = gpio_get(BUTTON_PIN);

        if (!button_state)
        {
            // Zustand ist LOW (0) -> Taster ist gedrückt
            printf("Button is pressed!\n");
        }
        else
        {
            // Zustand ist HIGH (1) -> Taster ist nicht gedrückt
        }

        // Kurze Pause, um CPU zu entlasten und Prellen zu vermeiden
        sleep_ms(100);
    }
}