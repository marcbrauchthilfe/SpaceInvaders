#include "game/game.h"
#include "game/gamestate.h"
#include "demos/display.h"
#include "hal/displays/st7735.h"
#include "pico/time.h"
#include <stdbool.h>

#define SCREEN_WIDTH 128
#define PLAYER_Y     150
#define PLAYER_WIDTH 10

#define ENEMY_ROWS 3
#define ENEMY_COLS 6
#define MAX_ENEMIES (ENEMY_ROWS * ENEMY_COLS)
#define MAX_BULLETS 5

/* =======================
   Enemy structure
   ======================= */
typedef struct {
    int x;
    int y;
    bool alive;
} Enemy;

/* =======================
   Bullet structure
   ======================= */
typedef struct {
    int x;
    int y;
    bool active;
} Bullet;

/* =======================
   Game state variables
   ======================= */
static Enemy enemies[MAX_ENEMIES];

static int player_x = 60;

/* Shooting */
static absolute_time_t last_shot_time;
static uint32_t shot_cooldown_us = 400000;
static Bullet bullets[MAX_BULLETS];

/* Enemy movement */
static int enemy_dir = 1;
static absolute_time_t last_enemy_move;
static uint32_t enemy_move_interval = 300000;

/* =======================
   Game Init
   ======================= */
void game_init(void)
{
    init_display();
    st7735_begin();

    st7735_fill_screen(st7735_rgb(0, 0, 0));

    set_state(GAMESTATE_MENU);
    st7735_draw_string(20, 40, "> SPACE INVADERS",
                       st7735_rgb(255, 255, 255),
                       st7735_rgb(0, 0, 0));

    st7735_draw_string(20, 60, "> LEFT = START",
                       st7735_rgb(255, 255, 255),
                       st7735_rgb(0, 0, 0));

    /* Init timers */
    last_shot_time = get_absolute_time();
    last_enemy_move = get_absolute_time();

    /* Init enemies */
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

/* =======================
   Game Update
   ======================= */
void game_update(int move_dir, int fire)
{
    if (get_state() != GAMESTATE_PLAYING)
        return;

    absolute_time_t now = get_absolute_time();

    /* -------- Player movement -------- */
    player_x += move_dir * 4;
    if (player_x < 0)
        player_x = 0;
    if (player_x > SCREEN_WIDTH - PLAYER_WIDTH)
        player_x = SCREEN_WIDTH - PLAYER_WIDTH;

    /* -------- Shooting -------- */
    if (fire) {
        if (absolute_time_diff_us(last_shot_time, now) >= shot_cooldown_us) {

            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullets[i].active) {
                    bullets[i].x = player_x + PLAYER_WIDTH / 2;
                    bullets[i].y = PLAYER_Y - 6;
                    bullets[i].active = true;
                    last_shot_time = now;
                    break;
                }
            }
        }
    }
    


    /* -------- Enemy movement -------- */
    if (absolute_time_diff_us(last_enemy_move, now) >= enemy_move_interval) {

        bool edge_hit = false;

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].alive) continue;

            enemies[i].x += enemy_dir * 2;

            if (enemies[i].x <= 0 || enemies[i].x >= SCREEN_WIDTH - 10) {
                edge_hit = true;
            }
        }

        if (edge_hit) {
            enemy_dir *= -1;
            for (int i = 0; i < MAX_ENEMIES; i++) {
                enemies[i].y += 5;
            }
        }

        last_enemy_move = now;
    }

    /* -------- Bullet movement -------- */
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        bullets[i].y -= 5;

        if (bullets[i].y < 0) {
            bullets[i].active = false;
        }
    }


    /* -------- Collision detection -------- */
   for (int b = 0; b < MAX_BULLETS; b++) {
        if (!bullets[b].active) continue;

        for (int e = 0; e < MAX_ENEMIES; e++) {
            if (!enemies[e].alive) continue;

            if (bullets[b].x >= enemies[e].x &&
                bullets[b].x <= enemies[e].x + 10 &&
                bullets[b].y >= enemies[e].y &&
                bullets[b].y <= enemies[e].y + 8) {

                enemies[e].alive = false;
                bullets[b].active = false;
                break;
            }
        }
    }


    /* -------- Render -------- */
    st7735_fill_screen(st7735_rgb(0, 0, 0));

    /* Player */
    st7735_fill_rect(player_x, PLAYER_Y,
                     PLAYER_WIDTH, 5,
                     st7735_rgb(255, 255, 255));

    /* Bullet */
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            st7735_fill_rect(
                bullets[i].x,
                bullets[i].y,
                2, 6,
                st7735_rgb(255, 0, 0)
            );
        }
    }


    /* Enemies */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            st7735_fill_rect(enemies[i].x,
                             enemies[i].y,
                             10, 6,
                             st7735_rgb(0, 255, 0));
        }
    }
}
