#ifndef GAME_H
#define GAME_H
#include "game/enemies.h"

void game_init(void);
void game_update(int move_dir, int fire);
void draw_game_screen(void);

#endif
