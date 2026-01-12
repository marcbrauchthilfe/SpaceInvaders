#include "game/gamestate.h"
#include <stdio.h>

static gamestate_t current_state = GAMESTATE_MENU;

void set_state(gamestate_t new_state) {
    current_state = new_state;
    print_state();
}

gamestate_t get_state(void) {
    return current_state;
}

void print_state(void) {
    switch (current_state) {
        case GAMESTATE_MENU:      printf("State: MENU\n"); break;
        case GAMESTATE_PLAYING:  printf("State: PLAYING\n"); break;
        case GAMESTATE_GAME_OVER:printf("State: GAME OVER\n"); break;
    }
}
