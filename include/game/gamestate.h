#ifndef GAMESTATE_H
#define GAMESTATE_H

typedef enum {
    GAMESTATE_MENU,
    GAMESTATE_PLAYING,
    GAMESTATE_GAME_OVER
} gamestate_t;

void set_state(gamestate_t new_state);
gamestate_t get_state(void);
void print_state(void);

#endif
