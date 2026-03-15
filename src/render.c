#include "frogger.h"

/* ── Color pair IDs ─────────────────────────────────────────────────────── */
#define CP_FROG      1
#define CP_CAR       2
#define CP_TRUCK     3
#define CP_LOG       4
#define CP_TURTLE    5
#define CP_WATER     6
#define CP_ROAD      7
#define CP_GRASS     8
#define CP_HOME      9
#define CP_STATUS   10
#define CP_DEAD     11
#define CP_HOME_OK  12
#define CP_MEDIAN   13

/* ── Initialise ncurses and colors ──────────────────────────────────────── */
void render_init(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        init_pair(CP_FROG,    COLOR_GREEN,   COLOR_BLACK);
        init_pair(CP_CAR,     COLOR_WHITE,   COLOR_RED);
        init_pair(CP_TRUCK,   COLOR_WHITE,   COLOR_MAGENTA);
        init_pair(CP_LOG,     COLOR_WHITE,   COLOR_YELLOW);
        init_pair(CP_TURTLE,  COLOR_GREEN,   COLOR_BLUE);
        init_pair(CP_WATER,   COLOR_CYAN,    COLOR_BLUE);
        init_pair(CP_ROAD,    COLOR_WHITE,   COLOR_BLACK);
        init_pair(CP_GRASS,   COLOR_BLACK,   COLOR_GREEN);
        init_pair(CP_HOME,    COLOR_WHITE,   COLOR_BLUE);
        init_pair(CP_STATUS,  COLOR_YELLOW,  COLOR_BLACK);
        init_pair(CP_DEAD,    COLOR_RED,     COLOR_BLACK);
        init_pair(CP_HOME_OK, COLOR_YELLOW,  COLOR_GREEN);
        init_pair(CP_MEDIAN,  COLOR_BLACK,   COLOR_GREEN);
    }
}

void render_cleanup(void)
{
    endwin();
}

/* ── Background rows ────────────────────────────────────────────────────── */
static void fill_row(int row, int pair, char ch)
{
    attron(COLOR_PAIR(pair));
    for (int c = 0; c < GAME_COLS; c++) {
        mvaddch(row, c, ch);
    }
    attroff(COLOR_PAIR(pair));
}

static void draw_backgrounds(void)
{
    fill_row(ROW_STATUS, CP_STATUS,  ' ');
    fill_row(ROW_HOME,   CP_WATER,   '~');
    for (int r = ROW_RIVER_TOP; r <= ROW_RIVER_BOT; r++) {
        fill_row(r, CP_WATER, '~');
    }
    fill_row(ROW_MEDIAN, CP_MEDIAN, ' ');
    for (int r = ROW_ROAD_TOP; r <= ROW_ROAD_BOT; r++) {
        fill_row(r, CP_ROAD, ' ');
    }
    fill_row(ROW_SAFE, CP_GRASS, ' ');
}

/* ── Road lane dividers ─────────────────────────────────────────────────── */
static void draw_road_dividers(void)
{
    attron(COLOR_PAIR(CP_ROAD));
    for (int r = ROW_ROAD_TOP; r < ROW_ROAD_BOT; r++) {
        for (int c = 2; c < GAME_COLS; c += 6) {
            mvaddch(r, c, '-');
            mvaddch(r, c + 1, '-');
        }
    }
    attroff(COLOR_PAIR(CP_ROAD));
}

/* ── Home lily pads ─────────────────────────────────────────────────────── */
static void draw_homes(const Game *g)
{
    for (int h = 0; h < HOME_COUNT; h++) {
        int col  = HOME_COLS[h];
        bool occ = g->homes[h].occupied;
        int pair = occ ? CP_HOME_OK : CP_HOME;

        attron(COLOR_PAIR(pair) | A_BOLD);
        if (occ) {
            /* Show a frog sitting in the pad */
            mvaddch(ROW_HOME, col,     '[');
            mvaddch(ROW_HOME, col + 1, '@');
            mvaddch(ROW_HOME, col + 2, '@');
            mvaddch(ROW_HOME, col + 3, ']');
        } else {
            mvaddch(ROW_HOME, col,     '[');
            mvaddch(ROW_HOME, col + 1, ' ');
            mvaddch(ROW_HOME, col + 2, ' ');
            mvaddch(ROW_HOME, col + 3, ']');
        }
        attroff(COLOR_PAIR(pair) | A_BOLD);
    }
}

/* ── Moving objects ─────────────────────────────────────────────────────── */
static void draw_obj(const Obj *o)
{
    if (!o->active) return;

    int pair;
    char left, body, right;

    switch (o->kind) {
        case OBJ_CAR:
            pair = CP_CAR;   left = '['; body = '='; right = ']'; break;
        case OBJ_TRUCK:
            pair = CP_TRUCK; left = '['; body = '#'; right = ']'; break;
        case OBJ_LOG:
            pair = CP_LOG;   left = '|'; body = '~'; right = '|'; break;
        case OBJ_TURTLE:
            pair = CP_TURTLE; left = 'o'; body = 'o'; right = 'o'; break;
        default:
            return;
    }

    attron(COLOR_PAIR(pair) | A_BOLD);
    int x = (int)o->x;
    for (int i = 0; i < o->width; i++) {
        int col = (x + i) % GAME_COLS;
        char ch;
        if (i == 0)              ch = left;
        else if (i == o->width - 1) ch = right;
        else                     ch = body;
        mvaddch(o->row, col, (chtype)ch);
    }
    attroff(COLOR_PAIR(pair) | A_BOLD);
}

/* ── Frog sprite ────────────────────────────────────────────────────────── */
static void draw_frog(const Game *g)
{
    if (g->frog.alive) {
        attron(COLOR_PAIR(CP_FROG) | A_BOLD);
        mvaddch(g->frog.row, g->frog.col, '@');
        attroff(COLOR_PAIR(CP_FROG) | A_BOLD);
    } else if (g->frog.death_timer > 0) {
        /* Flash during death animation */
        if ((g->frog.death_timer % 6) < 3) {
            attron(COLOR_PAIR(CP_DEAD) | A_BOLD);
            mvaddch(g->frog.row, g->frog.col, '*');
            attroff(COLOR_PAIR(CP_DEAD) | A_BOLD);
        }
    }
}

/* ── HUD / status bar ───────────────────────────────────────────────────── */
static void draw_status(const Game *g)
{
    attron(COLOR_PAIR(CP_STATUS) | A_BOLD);
    mvprintw(ROW_STATUS, 1, "FROGGER  Score:%06d", g->score);
    mvaddstr(ROW_STATUS, 25, "  Lives:");
    attroff(COLOR_PAIR(CP_STATUS) | A_BOLD);

    /* Draw frog icons for remaining lives */
    attron(COLOR_PAIR(CP_FROG) | A_BOLD);
    for (int i = 0; i < g->lives; i++) {
        mvaddch(ROW_STATUS, 34 + i * 2, '@');
    }
    attroff(COLOR_PAIR(CP_FROG) | A_BOLD);

    attron(COLOR_PAIR(CP_STATUS) | A_BOLD);
    mvprintw(ROW_STATUS, 46, " Level:%-3d", g->level);
    mvaddstr(ROW_STATUS, 57, " Arrows/WASD  [P]ause  [Q]uit");
    attroff(COLOR_PAIR(CP_STATUS) | A_BOLD);
}

/* ── Overlay messages ───────────────────────────────────────────────────── */
static void draw_game_over(const Game *g)
{
    int mr = (ROW_SAFE + ROW_STATUS) / 2 - 2;
    int mc = GAME_COLS / 2 - 14;

    attron(COLOR_PAIR(CP_DEAD) | A_BOLD);
    mvaddstr(mr,     mc, "  +---------------------------+  ");
    mvaddstr(mr + 1, mc, "  |        GAME  OVER         |  ");
    mvprintw(mr + 2, mc, "  |    Final Score: %06d     |  ", g->score);
    mvprintw(mr + 3, mc, "  |       Level reached: %-2d   |  ", g->level);
    mvaddstr(mr + 4, mc, "  |   Press  R  to restart    |  ");
    mvaddstr(mr + 5, mc, "  +---------------------------+  ");
    attroff(COLOR_PAIR(CP_DEAD) | A_BOLD);
}

static void draw_paused(void)
{
    int mr = (ROW_SAFE + ROW_STATUS) / 2;
    int mc = GAME_COLS / 2 - 11;

    attron(COLOR_PAIR(CP_STATUS) | A_BOLD | A_REVERSE);
    mvaddstr(mr,     mc, "  +---------------------+  ");
    mvaddstr(mr + 1, mc, "  |  PAUSED – press P   |  ");
    mvaddstr(mr + 2, mc, "  +---------------------+  ");
    attroff(COLOR_PAIR(CP_STATUS) | A_BOLD | A_REVERSE);
}

/* ── Main render entry point ────────────────────────────────────────────── */
void render_frame(const Game *g)
{
    erase();

    draw_backgrounds();
    draw_road_dividers();
    draw_homes(g);

    for (int i = 0; i < g->n_objs; i++) {
        draw_obj(&g->objs[i]);
    }

    draw_frog(g);
    draw_status(g);

    if (g->game_over) draw_game_over(g);
    if (g->paused)    draw_paused();

    refresh();
}
