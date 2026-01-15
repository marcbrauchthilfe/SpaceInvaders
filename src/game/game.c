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
#define MAX_BULLETS 5

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
static int player_x = 60;

/* Shooting */
static absolute_time_t last_shot_time;
static uint32_t shot_cooldown_us = 40000;
static Bullet bullets[MAX_BULLETS];

/*funktion init*/
static void draw_game_over_screen(void);

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

    srand(time_us_64()); // Seed mit aktueller Zeit                   

    /* Init timers */
    last_shot_time = get_absolute_time();

    enemies_init();
}

/* =======================
   Game Update
   ======================= */
void game_update(int move_dir, int fire)
{
    if (get_state() != GAMESTATE_PLAYING)
        return;
    else if (get_state() == GAMESTATE_GAME_OVER) {
    draw_game_over_screen();
        if (move_dir < 0) { // LEFT gedrückt
                game_init();               // reset Game
                set_state(GAMESTATE_PLAYING);
            }
        return;
}

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

    /* -------- Bullet movement -------- */
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        bullets[i].y -= 5;

        if (bullets[i].y < 0) {
            bullets[i].active = false;
        }
    }

    /* -------- Collision detection -------- */
    enemies_check_bullet_hits(player_x, PLAYER_Y, &bullets->active);
    if (enemies_player_hit(player_x, PLAYER_Y, PLAYER_WIDTH)) {
        set_state(GAMESTATE_GAME_OVER);
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
}

/* =======================
   Draw Game Over Screen
   ======================= */
static void draw_game_over_screen(void)
{
    st7735_fill_screen(st7735_rgb(0, 0, 0));

    st7735_draw_string(
        30, 40,
        "GAME OVER",
        st7735_rgb(255, 0, 0),
        st7735_rgb(0, 0, 0)
    );

    st7735_draw_string(
        10, 70,
        "LEFT = RESTART",
        st7735_rgb(255, 255, 255),
        st7735_rgb(0, 0, 0)
    );
}
