#ifndef ENEMIES_H
#define ENEMIES_H

#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Initialize enemies and enemy bullets */
void enemies_init(void);

/* Initialize enemies dynamically based on rows and columns */
void enemies_init_dynamic(int rows, int cols);

/* Update enemy positions and handle shooting */
void enemies_update(uint32_t move_interval, uint32_t shot_interval);

/* Draw enemies and enemy bullets */
void enemies_draw(void);

/* Check if a player bullet hits any enemy */
void enemies_check_bullet_hits(int bullet_x, int bullet_y, bool* bullet_active);

/* Check if player is hit by enemy bullets */
bool enemies_check_player_hit(int player_x, int player_y, int player_width, int player_height);

/* Check if all enemies are dead */
bool enemies_all_dead(void);

#endif /* ENEMIES_H */
