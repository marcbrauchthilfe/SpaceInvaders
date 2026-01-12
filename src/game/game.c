#include "game/game.h"
#include "game/gamestate.h"
#include "demos/display.h"
#include "hal/displays/st7735.h"

#define SCREEN_WIDTH 128
#define PLAYER_Y     150
#define PLAYER_WIDTH 10

static int player_x = 60;
static int bullet_x = -1;
static int bullet_y = -1;

void game_init(void)
{
    init_display();
    st7735_begin();
    // black background
    st7735_fill_screen(st7735_rgb(0, 0, 0));

    set_state(GAMESTATE_MENU);
    st7735_draw_string(20, 40, "> SPACE INVADERS", st7735_rgb(255, 255, 255), st7735_rgb(0, 0, 0));
    st7735_draw_string(20, 60, "> LEFT = START", st7735_rgb(255, 255, 255), st7735_rgb(0, 0, 0));
    st7735_draw_string(10, 80, "> Platzhalter", st7735_rgb(255, 255, 255), st7735_rgb(0, 0, 0));
    st7735_draw_string(10, 100, "> Platzhalter", st7735_rgb(255, 255, 255), st7735_rgb(0, 0, 0));
}

void game_update(int move_dir, int fire)
{
    if (get_state() != GAMESTATE_PLAYING)
        return;

    // Bewegung
    player_x += move_dir * 4;
    if (player_x < 0) player_x = 0;
    if (player_x > SCREEN_WIDTH - PLAYER_WIDTH)
        player_x = SCREEN_WIDTH - PLAYER_WIDTH;

    // Schuss
    if (fire && bullet_y < 0) {
        bullet_x = player_x + PLAYER_WIDTH / 2;
        bullet_y = PLAYER_Y;
    }

    // Update Bullet
    if (bullet_y >= 0) {
        bullet_y -= 5;
        if (bullet_y < 0)
            bullet_y = -1;
    }

    // Render

    // black background
    st7735_fill_screen(st7735_rgb(0, 0, 0));

    st7735_fill_rect(player_x, PLAYER_Y, PLAYER_WIDTH, 5, st7735_rgb(0, 0, 0));

    if (bullet_y >= 0) {
        st7735_fill_rect(bullet_x, bullet_y, 2, 6, st7735_rgb(0, 0, 0));
    }
}
