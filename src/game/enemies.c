#include "game/enemies.h"
#include "hal/displays/st7735.h"
#include "pico/time.h"
#include <stdlib.h>

#define SCREEN_WIDTH 128
#define MAX_ENEMY_ROWS 5
#define MAX_ENEMY_COLS 8
#define MAX_ENEMIES (MAX_ENEMY_ROWS * MAX_ENEMY_COLS)
#define MAX_ENEMY_BULLETS 6

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
static int active_enemies = 0;

static absolute_time_t last_enemy_move;
static absolute_time_t last_enemy_shot;

void enemies_init(void) {
    enemies_init_dynamic(1, 3);
}

void enemies_init_dynamic(int rows, int cols) {
    active_enemies = rows * cols;

    for (int i = 0; i < MAX_ENEMIES; i++)
        enemies[i].alive = false;

    int index = 0;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            enemies[index].x = 10 + c * 14;
            enemies[index].y = 20 + r * 12;
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

void enemies_update(uint32_t move_interval, uint32_t shot_interval) {
    absolute_time_t now = get_absolute_time();

    /* Bewegung */
    if (absolute_time_diff_us(last_enemy_move, now) >= move_interval) {
        bool edge = false;

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].alive) continue;
            enemies[i].x += enemy_dir * 2;
            if (enemies[i].x <= 0 || enemies[i].x >= SCREEN_WIDTH - 10)
                edge = true;
        }

        if (edge) {
            enemy_dir *= -1;
            for (int i = 0; i < MAX_ENEMIES; i++)
                if (enemies[i].alive)
                    enemies[i].y += 4;
        }

        last_enemy_move = now;
    }

    /* Schießen */
    if (absolute_time_diff_us(last_enemy_shot, now) >= shot_interval) {
        int tries = 10;
        while (tries--) {
            int i = rand() % MAX_ENEMIES;
            if (enemies[i].alive) {
                for (int b = 0; b < MAX_ENEMY_BULLETS; b++) {
                    if (!enemy_bullets[b].active) {
                        enemy_bullets[b].x = enemies[i].x + 4;
                        enemy_bullets[b].y = enemies[i].y + 6;
                        enemy_bullets[b].active = true;
                        last_enemy_shot = now;
                        return;
                    }
                }
            }
        }
    }

    /* Bullets bewegen */
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) continue;
        enemy_bullets[i].y += 4;
        if (enemy_bullets[i].y > 160)
            enemy_bullets[i].active = false;
    }
}

void enemies_draw(void) {
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].alive)
            st7735_fill_rect(enemies[i].x, enemies[i].y, 10, 6, st7735_rgb(0,255,0));

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
        if (enemy_bullets[i].active)
            st7735_fill_rect(enemy_bullets[i].x, enemy_bullets[i].y, 2, 6, st7735_rgb(255,255,0));
}

void enemies_check_bullet_hits(int x, int y, bool* active) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        if (x >= enemies[i].x && x <= enemies[i].x + 10 &&
            y >= enemies[i].y && y <= enemies[i].y + 6) {
            enemies[i].alive = false;
            active_enemies--;
            *active = false;
            return;
        }
    }
}

bool enemies_check_player_hit(int px, int py, int pw, int ph) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) continue;
        if (px < enemy_bullets[i].x + 2 &&
            px + pw > enemy_bullets[i].x &&
            py < enemy_bullets[i].y + 6 &&
            py + ph > enemy_bullets[i].y) {
            enemy_bullets[i].active = false;
            return true;
        }
    }
    return false;
}

bool enemies_all_dead(void) {
    return active_enemies <= 0;
}
