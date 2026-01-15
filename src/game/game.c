#include "game/game.h"
#include "game/gamestate.h"
#include "demos/display.h"
#include "hal/displays/st7735.h"
#include "pico/time.h"
#include <stdbool.h>
#include <stdlib.h>

#define SCREEN_WIDTH 128
#define PLAYER_Y     150
#define PLAYER_WIDTH 10

#define ENEMY_ROWS 3
#define ENEMY_COLS 6
#define MAX_ENEMIES (ENEMY_ROWS * ENEMY_COLS)
#define MAX_BULLETS 50
#define MAX_ENEMY_BULLETS 5

/* =======================
   Structures
   ======================= */
typedef struct {
    int x;
    int y;
    bool alive;
} Enemy;

typedef struct {
    int x;
    int y;
    bool active;
} Bullet;

/* =======================
   Game variables
   ======================= */
static Enemy enemies[MAX_ENEMIES];
static Bullet bullets[MAX_BULLETS];
static Bullet enemy_bullets[MAX_ENEMY_BULLETS];

static int player_x = 60;
static int enemy_dir = 1;

/* Timing */
static absolute_time_t last_shot_time;
static absolute_time_t last_enemy_move;
static absolute_time_t last_enemy_shot;

static uint32_t shot_cooldown_us = 40000;
static uint32_t enemy_move_interval = 300000;
static uint32_t enemy_shot_interval_us = 800000;

/* UI flags (gegen Flackern!) */
static bool menu_drawn = false;
static bool game_over_drawn = false;

/* =======================
   Forward declarations
   ======================= */
static void draw_menu(void);
static void draw_game_over_screen(void);

/* =======================
   Game Init
   ======================= */
void game_init(void)
{
    init_display();
    st7735_begin();

    srand(time_us_64());

    player_x = 60;
    enemy_dir = 1;

    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) enemy_bullets[i].active = false;

    int idx = 0;
    for (int r = 0; r < ENEMY_ROWS; r++) {
        for (int c = 0; c < ENEMY_COLS; c++) {
            enemies[idx].x = 10 + c * 18;
            enemies[idx].y = 20 + r * 15;
            enemies[idx].alive = true;
            idx++;
        }
    }

    last_shot_time = get_absolute_time();
    last_enemy_move = get_absolute_time();
    last_enemy_shot = get_absolute_time();

    menu_drawn = false;
    game_over_drawn = false;

    set_state(GAMESTATE_MENU);
}

/* =======================
   Game Update
   ======================= */
void game_update(int move_dir, int fire)
{
    absolute_time_t now = get_absolute_time();

    /* ---------- GAME OVER ---------- */
    if (get_state() == GAMESTATE_GAME_OVER) {

        if (!game_over_drawn) {
            draw_game_over_screen();
            game_over_drawn = true;
        }

        if (move_dir < 0) { // LEFT
            game_init();          // Reset
        }
        return;
    }

    /* ---------- MENU ---------- */
    if (get_state() == GAMESTATE_MENU) {

        if (!menu_drawn) {
            draw_menu();
            menu_drawn = true;
        }

        if (move_dir < 0) { // LEFT = START
            st7735_fill_screen(st7735_rgb(0, 0, 0));
            set_state(GAMESTATE_PLAYING);
        }
        return;
    }

    /* ---------- PLAYING ---------- */

    /* Player movement */
    player_x += move_dir * 4;
    if (player_x < 0) player_x = 0;
    if (player_x > SCREEN_WIDTH - PLAYER_WIDTH)
        player_x = SCREEN_WIDTH - PLAYER_WIDTH;

    /* Player shooting */
    if (fire && absolute_time_diff_us(last_shot_time, now) >= shot_cooldown_us) {
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

    /* Enemy movement */
    if (absolute_time_diff_us(last_enemy_move, now) >= enemy_move_interval) {
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
                enemies[i].y += 5;
        }

        last_enemy_move = now;
    }

    /* Enemy shooting */
    if (absolute_time_diff_us(last_enemy_shot, now) >= enemy_shot_interval_us) {
        int shooter = rand() % MAX_ENEMIES;
        if (enemies[shooter].alive) {
            for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
                if (!enemy_bullets[i].active) {
                    enemy_bullets[i].x = enemies[shooter].x + 5;
                    enemy_bullets[i].y = enemies[shooter].y + 6;
                    enemy_bullets[i].active = true;
                    last_enemy_shot = now;
                    break;
                }
            }
        }
    }

    /* Bullet movement */
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y -= 5;
            if (bullets[i].y < 0) bullets[i].active = false;
        }
    }

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (enemy_bullets[i].active) {
            enemy_bullets[i].y += 4;
            if (enemy_bullets[i].y > 160)
                enemy_bullets[i].active = false;
        }
    }

    /* Collisions */
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
            }
        }
    }

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) continue;
        if (enemy_bullets[i].x >= player_x &&
            enemy_bullets[i].x <= player_x + PLAYER_WIDTH &&
            enemy_bullets[i].y >= PLAYER_Y) {

            set_state(GAMESTATE_GAME_OVER);
            return;
        }
    }

    /* ---------- Render ---------- */
    st7735_fill_screen(st7735_rgb(0, 0, 0));

    st7735_fill_rect(player_x, PLAYER_Y, PLAYER_WIDTH, 5,
                     st7735_rgb(255, 255, 255));

    for (int i = 0; i < MAX_BULLETS; i++)
        if (bullets[i].active)
            st7735_fill_rect(bullets[i].x, bullets[i].y, 2, 6,
                             st7735_rgb(255, 0, 0));

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
        if (enemy_bullets[i].active)
            st7735_fill_rect(enemy_bullets[i].x, enemy_bullets[i].y, 2, 6,
                             st7735_rgb(255, 255, 0));

    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].alive)
            st7735_fill_rect(enemies[i].x, enemies[i].y, 10, 6,
                             st7735_rgb(0, 255, 0));
}

/* =======================
   UI Screens
   ======================= */
static void draw_menu(void)
{
    st7735_fill_screen(st7735_rgb(0, 0, 0));
    st7735_draw_string(20, 40, "SPACE INVADERS",
                       st7735_rgb(255,255,255), 0);
    st7735_draw_string(20, 60, "LEFT = START",
                       st7735_rgb(255,255,255), 0);
}

static void draw_game_over_screen(void)
{
    st7735_fill_screen(st7735_rgb(0, 0, 0));
    st7735_draw_string(30, 40, "GAME OVER",
                       st7735_rgb(255,0,0), 0);
    st7735_draw_string(10, 70, "LEFT = MENU",
                       st7735_rgb(255,255,255), 0);
}
