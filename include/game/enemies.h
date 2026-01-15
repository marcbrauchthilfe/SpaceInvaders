#ifndef ENEMIES_H
#define ENEMIES_H

#include <stdbool.h>

void enemies_init(void);

void enemies_update(void);

void enemies_draw(void);

void enemies_check_bullet_hits(int bullet_x, int bullet_y, bool* bullet_aktive);

bool enemies_player_hit(int player_x, int player_y, int player_width);

#endif