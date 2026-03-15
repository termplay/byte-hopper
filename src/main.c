#include "frogger.h"
#include <stdlib.h>
#include <time.h>

#define TICK_MS 80   /* ~12 ticks/sec */

int main(void)
{
    srand((unsigned int)time(NULL));

    render_init();

    Game g;
    game_init(&g);

    bool quit = false;
    while (!quit) {
        int key = getch();

        switch (key) {
            case 'q': case 'Q':
                quit = true;
                break;
            case 'r': case 'R':
                if (g.game_over) game_init(&g);
                break;
            case 'p': case 'P':
                if (!g.game_over) g.paused = !g.paused;
                break;
            case KEY_UP:    case 'w': case 'W':
                game_move_frog(&g, -1,  0);
                break;
            case KEY_DOWN:  case 's': case 'S':
                game_move_frog(&g,  1,  0);
                break;
            case KEY_LEFT:  case 'a': case 'A':
                game_move_frog(&g,  0, -1);
                break;
            case KEY_RIGHT: case 'd': case 'D':
                game_move_frog(&g,  0,  1);
                break;
            default:
                break;
        }

        game_tick(&g);
        render_frame(&g);
        napms(TICK_MS);
    }

    render_cleanup();
    return 0;
}
