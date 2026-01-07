#include "game/gamestate.h"
#include <stdio.h>

gamestate_t current_state = GAMESTATE_MENU;

void set_state(gamestate_t new_state) {
    current_state = new_state;
}

void print_state(void) {
    switch (current_state) {
        case GAMESTATE_MENU:
            printf("State: MENU\n");
            break;
        case GAMESTATE_PLAYING:
            printf("State: PLAYING\n");
            break;
        case GAMESTATE_GAME_OVER:
            printf("State: GAME OVER\n");
            break;
    }
}


gamestate_t get_state(gamestate_t) {
    return current_state;
}