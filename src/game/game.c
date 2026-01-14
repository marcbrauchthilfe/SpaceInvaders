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
#define MAX_BULLETS 5
#define MAX_ENEMY_BULLETS 5

/* =======================
   Enemy structure
   ======================= */
typedef struct {
    int x;
    int y;
    bool alive;
} Enemy;

/* =======================
   Enemy Bullet structure
   ======================= */
typedef struct {
    int x;
    int y;
    bool active;
} EnemyBullet;

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
static uint32_t shot_cooldown_us = 40000;
static Bullet bullets[MAX_BULLETS];

/* Enemy */
static int enemy_dir = 1;
static absolute_time_t last_enemy_move;
static uint32_t enemy_move_interval = 300000;
static EnemyBullet enemy_bullets[MAX_ENEMY_BULLETS];
static absolute_time_t last_enemy_shot;
static uint32_t enemy_shot_interval_us = 800000; // 0.8s

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
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
    enemy_bullets[i].active = false;
    }
    last_enemy_shot = get_absolute_time();

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

    /* -------- Enemy Shooting -------- */
    if (absolute_time_diff_us(last_enemy_shot, now) >= enemy_shot_interval_us) {

        /* zufälligen lebenden Gegner wählen */
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
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) continue;

        if (enemy_bullets[i].x >= player_x &&
            enemy_bullets[i].x <= player_x + PLAYER_WIDTH &&
            enemy_bullets[i].y >= PLAYER_Y &&
            enemy_bullets[i].y <= PLAYER_Y + 5) {

            enemy_bullets[i].active = false;

            /* TODO: Leben abziehen / Game Over */
            set_state(GAMESTATE_GAME_OVER);
        }
    }


    /* -------- Enemy Bullet movement -------- */
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) continue;

        enemy_bullets[i].y += 4;

        if (enemy_bullets[i].y > 160) {
            enemy_bullets[i].active = false;
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

    /* Enemy Bullets */
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (enemy_bullets[i].active) {
            st7735_fill_rect(
                enemy_bullets[i].x,
                enemy_bullets[i].y,
                2, 6,
                st7735_rgb(255, 255, 0)
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
