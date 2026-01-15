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

typedef struct {
    int x, y;
    bool active;
} Bullet;

static int player_x = 60;
static Bullet bullets[MAX_BULLETS];
static absolute_time_t last_shot_time;
static uint32_t shot_cooldown_us = 40000;

static void draw_game_over_screen(void) {
    st7735_fill_screen(st7735_rgb(0,0,0));
    st7735_draw_string(30,40,"GAME OVER", st7735_rgb(255,0,0), st7735_rgb(0,0,0));
    st7735_draw_string(10,70,"LEFT = RESTART", st7735_rgb(255,255,255), st7735_rgb(0,0,0));
}

void game_init(void) {
    init_display();
    st7735_begin();
    enemies_init();
    st7735_fill_screen(st7735_rgb(0,0,0));

    set_state(GAMESTATE_MENU);
    st7735_draw_string(20,40,"> SPACE INVADERS", st7735_rgb(255,255,255), st7735_rgb(0,0,0));
    st7735_draw_string(20,60,"> LEFT = START", st7735_rgb(255,255,255), st7735_rgb(0,0,0));

    srand(time_us_64());
    last_shot_time = get_absolute_time();

    for(int i=0;i<MAX_BULLETS;i++) bullets[i].active = false;
}

void game_update(int move_dir, int fire) {
    if (get_state() == GAMESTATE_GAME_OVER) {
        draw_game_over_screen();
        if (move_dir < 0) game_init();
        return;
    }

    if (get_state() == GAMESTATE_MENU) {
        if (move_dir < 0) set_state(GAMESTATE_PLAYING);
        return;
    }

    /* Player movement */
    player_x += move_dir * 4;
    if (player_x < 0) player_x = 0;
    if (player_x > SCREEN_WIDTH - PLAYER_WIDTH) player_x = SCREEN_WIDTH - PLAYER_WIDTH;

    /* Shooting */
    absolute_time_t now = get_absolute_time();
    if (fire && absolute_time_diff_us(last_shot_time, now) >= shot_cooldown_us) {
        for (int i=0;i<MAX_BULLETS;i++) {
            if(!bullets[i].active){
                bullets[i].x = player_x + PLAYER_WIDTH/2;
                bullets[i].y = PLAYER_Y - 6;
                bullets[i].active = true;
                last_shot_time = now;
                break;
            }
        }
    }

    /* Move bullets */
    for(int i=0;i<MAX_BULLETS;i++){
        if(!bullets[i].active) continue;
        bullets[i].y -= 5;
        if(bullets[i].y < 0) bullets[i].active = false;
    }

    /* Update enemies & bullets */
    enemies_update();

    /* Check bullet collisions with enemies */
    for(int i=0;i<MAX_BULLETS;i++){
        if(bullets[i].active)
            enemies_check_bullet_hits(bullets[i].x, bullets[i].y, &bullets[i].active);
    }

    /* Check if player is hit */
    if(enemies_check_player_hit(player_x, PLAYER_Y, PLAYER_WIDTH, 5)) {
        set_state(GAMESTATE_GAME_OVER);
    }

    /* Render */
    st7735_fill_screen(st7735_rgb(0,0,0));
    st7735_fill_rect(player_x, PLAYER_Y, PLAYER_WIDTH, 5, st7735_rgb(255,255,255));

    for(int i=0;i<MAX_BULLETS;i++){
        if(bullets[i].active)
            st7735_fill_rect(bullets[i].x, bullets[i].y, 2, 6, st7735_rgb(255,0,0));
    }

    enemies_draw();
}
