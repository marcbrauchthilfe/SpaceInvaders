#include "include/demos/Abgabe_09.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "game/game.h"
#include "game/gamestate.h"
#include "hal/controls/joystick.h"
#include "demos/joystick.h"

#define LEFT_BUTTON_PIN   15
#define BOTTOM_BUTTON_PIN 14
#define TOP_BUTTON_PIN    10
#define RIGHT_BUTTON_PIN  11

void abgabe_09_execute(void)
{
    joystick_init_simple_center();
    joystick_event_t event;


    gpio_init_mask(
        (1<<LEFT_BUTTON_PIN) |
        (1<<RIGHT_BUTTON_PIN) |
        (1<<TOP_BUTTON_PIN) |
        (1<<BOTTOM_BUTTON_PIN)
    );

    gpio_set_dir(LEFT_BUTTON_PIN, GPIO_IN);
    gpio_set_dir(RIGHT_BUTTON_PIN, GPIO_IN);
    gpio_set_dir(TOP_BUTTON_PIN, GPIO_IN);
    gpio_set_dir(BOTTOM_BUTTON_PIN, GPIO_IN);

    gpio_pull_up(LEFT_BUTTON_PIN);
    gpio_pull_up(RIGHT_BUTTON_PIN);
    gpio_pull_up(TOP_BUTTON_PIN);
    gpio_pull_up(BOTTOM_BUTTON_PIN);

    while (true)
    {
        joystick_read(&event);

        int move = 0;
        int fire = 0;

        if (event.x_norm < -0.5f) move = 1;
        if (event.x_norm >  0.5f) move =  -1;

        if (!gpio_get(LEFT_BUTTON_PIN))  move = -1;
        if (!gpio_get(RIGHT_BUTTON_PIN)) move =  1;
        if (!gpio_get(TOP_BUTTON_PIN))   fire = 1;

        if (get_state() == GAMESTATE_MENU && move != 0) {
            set_state(GAMESTATE_PLAYING);
        }

        game_update(move, fire);
        sleep_ms(50);
    }
}
