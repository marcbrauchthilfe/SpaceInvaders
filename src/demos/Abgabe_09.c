#include "include/demos/Abgabe_09.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include "game/gamestate.h"

#define LEFT_BUTTON_PIN 15      //left
#define BOTTOM_BUTTON_PIN 14    //bottom
#define TOP_BUTTON_PIN 10       //top
#define RIGHT_BUTTON_PIN 11     //right

void abgabe_09_execute(void)
{
    // Initialisiere Buttons
    gpio_init(LEFT_BUTTON_PIN);
    gpio_init(BOTTOM_BUTTON_PIN);
    gpio_init(TOP_BUTTON_PIN);
    gpio_init(RIGHT_BUTTON_PIN);

    // Setze den Pin als Eingang (Input)
    gpio_set_dir(LEFT_BUTTON_PIN, GPIO_IN);
    gpio_set_dir(BOTTOM_BUTTON_PIN, GPIO_IN);
    gpio_set_dir(TOP_BUTTON_PIN, GPIO_IN);
    gpio_set_dir(RIGHT_BUTTON_PIN, GPIO_IN);

    // Aktiviere den internen Pull-Up-Widerstand.
    // Der Pin ist standardmäßig HIGH (1).
    // Er wird LOW (0), wenn der Taster gedrückt wird (Verbindung mit GND).
    gpio_pull_up(LEFT_BUTTON_PIN);
    gpio_pull_up(BOTTOM_BUTTON_PIN);
    gpio_pull_up(TOP_BUTTON_PIN);
    gpio_pull_up(RIGHT_BUTTON_PIN);

    printf("Read button state started (Left=%d, Bottom=%d, Top=%d, Right=%d).\n",
           LEFT_BUTTON_PIN, BOTTOM_BUTTON_PIN, TOP_BUTTON_PIN, RIGHT_BUTTON_PIN);


    // Endlosschleife
    while (true)
    {
        bool left_Button_state   = gpio_get(LEFT_BUTTON_PIN);
        bool bottom_Button_state = gpio_get(BOTTOM_BUTTON_PIN);
        bool top_Button_state    = gpio_get(TOP_BUTTON_PIN);
        bool right_Button_state  = gpio_get(RIGHT_BUTTON_PIN);

        if (!left_Button_state)
        {
            printf("left Button is pressed!\n");
            switch (get_state())
            {
            case GAMESTATE_MENU:
                // start game
                break;
            case GAMESTATE_GAME_OVER:
                // restart game
                break;
            default:
                break;
            }
            sleep_ms(200);
        }
        else if (!bottom_Button_state)
        {
            printf("bottom Button is pressed!\n");
            sleep_ms(200);
        }
        else if (!top_Button_state)
        {
            printf("top Button is pressed!\n");
            if (get_state() == GAMESTATE_PLAYING)
            {
                // fire bullet
            }
            
            sleep_ms(200);
        }
        else if (!right_Button_state)
        {
            printf("right Button is pressed!\n");
            switch (get_state())
            {
            case  GAMESTATE_MENU:
                // show controlls 
                break;
            case GAMESTATE_GAME_OVER:
                // back to menu
                break;
            default:
                break;
            }
        }
        sleep_ms(100);
    }
}