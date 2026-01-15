#include "game/game.h"
#include "game/gamestate.h"
#include "demos/display.h"
#include "hal/displays/st7735.h"
#include "pico/time.h"
#include <stdbool.h>
#include <stdlib.h>
#include "game/enemies.h"

#define SCREEN_WIDTH 128
#define PLAYER_Y     150
#define PLAYER_WIDTH 10
#define MAX_BULLETS  50
#define FRAME_INTERVAL_US 16666 // ~60 FPS

/* =======================
   Bullet
   ======================= */
typedef struct {
    int x, y;
    bool active;
} Bullet;

/* =======================
   Difficulty
   ======================= */
typedef struct {
    int wave;
    int enemy_rows;
    int enemy_cols;
    uint32_t enemy_move_interval;
    uint32_t enemy_shot_interval;
} Difficulty;

static Difficulty diff;

/* =======================
   Game variables
   ======================= */
static int player_x = 60;
static Bullet bullets[MAX_BULLETS];

static absolute_time_t last_shot_time;
static absolute_time_t last_frame_time;

static uint32_t shot_cooldown_us = 250000;

/* UI flags */
static bool menu_drawn = false;
static bool game_over_drawn = false;

/* =======================
   Forward declarations
   ======================= */
static void draw_menu(void);
static void draw_game_over_screen(void);
static void increase_difficulty(void);

/* =======================
   Game Init
   ======================= */
void game_init(void) {
    init_display();
    st7735_begin();
    st7735_fill_screen(st7735_rgb(0,0,0));

    srand(time_us_64());

    player_x = 60;

    for (int i = 0; i < MAX_BULLETS; i++)
        bullets[i].active = false;

    last_shot_time = get_absolute_time();
    last_frame_time = get_absolute_time();

    menu_drawn = false;
    game_over_drawn = false;

    /* Difficulty Startwerte */
    diff.wave = 1;
    diff.enemy_rows = 1;
    diff.enemy_cols = 3;
    diff.enemy_move_interval = 300000;
    diff.enemy_shot_interval = 800000;

    enemies_init_dynamic(diff.enemy_rows, diff.enemy_cols);

    set_state(GAMESTATE_MENU);
}

/* =======================
   Game Update
   ======================= */
void game_update(int move_dir, int fire) {
    absolute_time_t now = get_absolute_time();

    if (absolute_time_diff_us(last_frame_time, now) < FRAME_INTERVAL_US)
        return;
    last_frame_time = now;

    /* ---------- GAME OVER ---------- */
    if (get_state() == GAMESTATE_GAME_OVER) {
        if (!game_over_drawn) {
            draw_game_over_screen();
            game_over_drawn = true;
        }
        if (move_dir < 0)
            game_init();
        return;
    }

    /* ---------- MENU ---------- */
    if (get_state() == GAMESTATE_MENU) {
        if (!menu_drawn) {
            draw_menu();
            menu_drawn = true;
        }
        if (move_dir < 0) {
            st7735_fill_screen(st7735_rgb(0,0,0));
            set_state(GAMESTATE_PLAYING);
        }
        return;
    }

    /* ---------- PLAYING ---------- */

    /* Player movement */
    int prev_x = player_x;
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

    /* Move bullets */
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        bullets[i].y -= 5;
        if (bullets[i].y < 0)
            bullets[i].active = false;
    }

    /* Update enemies */
    enemies_update(diff.enemy_move_interval, diff.enemy_shot_interval);

    /* Bullet hits */
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active)
            enemies_check_bullet_hits(
                bullets[i].x,
                bullets[i].y,
                &bullets[i].active
            );
    }

    /* Player hit */
    if (enemies_check_player_hit(player_x, PLAYER_Y, PLAYER_WIDTH, 5)) {
        set_state(GAMESTATE_GAME_OVER);
        return;
    }

    /* ---------- Rendering ---------- */
    st7735_fill_rect(prev_x, PLAYER_Y, PLAYER_WIDTH, 5, st7735_rgb(0,0,0));
    st7735_fill_rect(player_x, PLAYER_Y, PLAYER_WIDTH, 5, st7735_rgb(255,255,255));

    for (int i = 0; i < MAX_BULLETS; i++)
        if (bullets[i].active)
            st7735_fill_rect(bullets[i].x, bullets[i].y, 2, 6, st7735_rgb(255,0,0));

    enemies_draw();

    /* ---------- Next Wave ---------- */
    if (enemies_all_dead()) {
        increase_difficulty();
    }
}

/* =======================
   Difficulty Scaling
   ======================= */
static void increase_difficulty(void) {
    diff.wave++;

    if (diff.enemy_rows < 5) diff.enemy_rows++;
    if (diff.enemy_cols < 8) diff.enemy_cols++;

    if (diff.enemy_move_interval > 100000)
        diff.enemy_move_interval -= 20000;

    if (diff.enemy_shot_interval > 300000)
        diff.enemy_shot_interval -= 50000;

    enemies_init_dynamic(diff.enemy_rows, diff.enemy_cols);
}

/* =======================
   UI Screens
   ======================= */
static void draw_menu(void) {
    st7735_fill_screen(st7735_rgb(0,0,0));
    st7735_draw_string(20, 40, "SPACE INVADERS", st7735_rgb(255,255,255), 0);
    st7735_draw_string(20, 60, "LEFT = START", st7735_rgb(255,255,255), 0);
}

static void draw_game_over_screen(void) {
    st7735_fill_screen(st7735_rgb(0,0,0));
    st7735_draw_string(30, 40, "GAME OVER", st7735_rgb(255,0,0), 0);
    st7735_draw_string(10, 70, "LEFT = RESTART", st7735_rgb(255,255,255), 0);
}
