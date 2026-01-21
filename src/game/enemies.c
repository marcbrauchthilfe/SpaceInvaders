#include "game/enemies.h"
#include "hal/displays/st7735.h"
#include "pico/time.h"
#include <stdlib.h>
#include <stdbool.h>

#define SCREEN_WIDTH 128
#define ENEMY_ROWS 3
#define ENEMY_COLS 6
#define MAX_ENEMIES (ENEMY_ROWS * ENEMY_COLS)
#define MAX_ENEMY_BULLETS 5

typedef struct {
    int x, y;
    bool alive;
} Enemy;

typedef struct {
    int x, y;
    bool active;
} EnemyBullet;

static Enemy enemies[MAX_ENEMIES];
static EnemyBullet enemy_bullets[MAX_ENEMY_BULLETS];
static int enemy_dir = 1;
static absolute_time_t last_enemy_move;
static absolute_time_t last_enemy_shot;
static uint32_t enemy_move_interval = 300000;
static uint32_t enemy_shot_interval_us = 800000;
static int current_wave = 1;
static int score = 0;
static int enemy_speed = 1;
static int enemy_shoot_delay = 2000;


void enemies_init(void) {
    int index = 0;
    for (int row = 0; row < ENEMY_ROWS; row++) {
        for (int col = 0; col < ENEMY_COLS; col++) {
            enemies[index].x = 10 + col * 18;
            enemies[index].y = 20 + row * 15;
            enemies[index].alive = true;
            index++;
        }
    }

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
        enemy_bullets[i].active = false;

    enemy_dir = 1;
    last_enemy_move = get_absolute_time();
    last_enemy_shot = get_absolute_time();
}

void enemies_update(void) {
    absolute_time_t now = get_absolute_time();

    // Enemy Movement
    if (absolute_time_diff_us(last_enemy_move, now) >= enemy_move_interval) {
        bool edge_hit = false;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].alive) continue;
            enemies[i].x += enemy_dir * 2;
            if (enemies[i].x <= 0 || enemies[i].x >= SCREEN_WIDTH - 10)
                edge_hit = true;
        }
        if (edge_hit) {
            enemy_dir *= -1;
            for (int i = 0; i < MAX_ENEMIES; i++)
                enemies[i].y += 5;
        }
        last_enemy_move = now;
    }

    // Enemy Shooting
    if (absolute_time_diff_us(last_enemy_shot, now) >= enemy_shot_interval_us) {
        int shooter = -1;
        for (int tries = 0; tries < 10; tries++) {
            int i = rand() % MAX_ENEMIES;
            if (enemies[i].alive) {
                shooter = i;
                break;
            }
        }

        if (shooter >= 0) {
            for (int b = 0; b < MAX_ENEMY_BULLETS; b++) {
                if (!enemy_bullets[b].active) {
                    enemy_bullets[b].x = enemies[shooter].x + 5;
                    enemy_bullets[b].y = enemies[shooter].y + 6;
                    enemy_bullets[b].active = true;
                    last_enemy_shot = now;
                    break;
                }
            }
        }
    }

    // Move Enemy Bullets
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) continue;
        enemy_bullets[i].y += 4;
        if (enemy_bullets[i].y > 160) enemy_bullets[i].active = false;
    }
}

void enemies_draw(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive)
            st7735_fill_rect(enemies[i].x, enemies[i].y, 10, 6, st7735_rgb(0, 255, 0));
    }
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (enemy_bullets[i].active)
            st7735_fill_rect(enemy_bullets[i].x, enemy_bullets[i].y, 2, 6, st7735_rgb(255, 255, 0));
    }
}

void enemies_check_bullet_hits(int bullet_x, int bullet_y, bool* bullet_active) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        if (bullet_x >= enemies[i].x && bullet_x <= enemies[i].x + 10 &&
            bullet_y >= enemies[i].y && bullet_y <= enemies[i].y + 6) {
            enemies[i].alive = false;
            *bullet_active = false;
            break;
        }
    }
}

bool enemies_check_player_hit(int player_x, int player_y, int player_width, int player_height) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) continue;
        if (player_x < enemy_bullets[i].x + 2 &&
            player_x + player_width > enemy_bullets[i].x &&
            player_y < enemy_bullets[i].y + 6 &&
            player_y + player_height > enemy_bullets[i].y) {
            enemy_bullets[i].active = false;
            return true;
        }
    }
    return false;

}
bool enemies_all_dead(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            return false;
        }
    }
    return true;
}

void enemies_spawn_wave(int wave) {
    enemy_speed = 1 + wave;          // schneller
    enemy_shoot_delay = 2000 - wave * 150; // aggressiver
    if (enemy_shoot_delay < 400) enemy_shoot_delay = 400;

    int index = 0;
    for (int row = 0; row < ENEMY_ROWS; row++) {
        for (int col = 0; col < ENEMY_COLS; col++) {
            enemies[index].x = 10 + col * 18;
            enemies[index].y = 20 + row * 15;
            enemies[index].alive = true;
            index++;
        }
    }
}


