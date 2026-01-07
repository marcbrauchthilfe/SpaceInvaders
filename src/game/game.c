#include <stdio.h>
#include "game/game.h"
#include "demos/display.h"



void game_init(void)
{
    init_display();
    st7735_begin();

    display_draw_menu();
}
