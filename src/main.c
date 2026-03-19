#include "frogger.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define TICK_MS 80   /* ~12 ticks/sec */
#define NAME_MAXLEN 15  /* must fit in HighScore.name[16] with NUL */

static void show_start_screen(void)
{
    HighScore scores[MAX_HIGHSCORES];
    int n = highscore_load(scores, MAX_HIGHSCORES);

    nodelay(stdscr, FALSE); /* blocking input for menu */
    render_start_screen(scores, n);

    /* Wait for ENTER */
    int key;
    while ((key = getch()) != '\n' && key != KEY_ENTER) {
        if (key == 'q' || key == 'Q') {
            render_cleanup();
            exit(0);
        }
    }

    nodelay(stdscr, TRUE); /* non-blocking for gameplay */
}

static void prompt_name(char *buf, int buflen, int score, int level)
{
    buf[0] = '\0';
    int len = 0;
    int maxlen = buflen - 1;
    if (maxlen > NAME_MAXLEN) maxlen = NAME_MAXLEN;

    nodelay(stdscr, FALSE); /* blocking input */

    render_name_input(score, level, buf, len);

    int key;
    while ((key = getch()) != '\n' && key != KEY_ENTER) {
        if ((key == KEY_BACKSPACE || key == 127 || key == 8) && len > 0) {
            buf[--len] = '\0';
        } else if (isprint(key) && len < maxlen && key != ' ') {
            /* No spaces — the file format uses space as delimiter */
            buf[len++] = (char)key;
            buf[len]   = '\0';
        }
        render_name_input(score, level, buf, len);
    }

    if (len == 0) {
        snprintf(buf, (size_t)buflen, "ANON");
    }

    nodelay(stdscr, TRUE); /* back to non-blocking */
}

static void save_score(const Game *g)
{
    if (g->score <= 0) return;

    HighScore scores[MAX_HIGHSCORES];
    int n = highscore_load(scores, MAX_HIGHSCORES);

    if (highscore_qualifies(scores, n, MAX_HIGHSCORES, g->score)) {
        char name[16];
        prompt_name(name, (int)sizeof(name), g->score, g->level);
        n = highscore_insert(scores, n, MAX_HIGHSCORES, g->score, g->level, name);
        highscore_save(scores, n);
    }
}

int main(void)
{
    srand((unsigned int)time(NULL));

    render_init();
    show_start_screen();

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
                if (g.game_over) {
                    save_score(&g);
                    show_start_screen();
                    game_init(&g);
                }
                break;
            case 'p': case 'P':
                if (!g.game_over && g.level_clear_timer <= 0)
                    g.paused = !g.paused;
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

    save_score(&g);
    render_cleanup();
    return 0;
}
