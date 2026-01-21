#ifndef ENEMIES_H
#define ENEMIES_H

#include <stdbool.h>

/* Initialize enemies and enemy bullets */
void enemies_init(void);

/* Update enemy positions and bullets */
void enemies_update(void);

/* Draw enemies and enemy bullets */
void enemies_draw(void);

/* Check if a player bullet hits any enemy */
void enemies_check_bullet_hits(int bullet_x, int bullet_y, bool* bullet_active);

/* Check if player is hit by enemy bullets */
bool enemies_check_player_hit(int player_x, int player_y, int player_width, int player_height);

/* Check if all enemies are dead */
bool enemies_all_dead(void);

/* Spawn a new wave of enemies */
void enemies_spawn_wave(int wave);

#endif /* ENEMIES_H */
