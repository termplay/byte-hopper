#ifndef FROGGER_FROGGER_H
#define FROGGER_FROGGER_H

#include <stdbool.h>
#include <ncurses.h>

/* ── Screen layout (rows, 0-indexed from top) ───────────────────────────── */
#define GAME_COLS       80
#define GAME_ROWS       15

#define ROW_STATUS       0   /* score/lives bar */
#define ROW_HOME         1   /* five lily-pad goals */
#define ROW_RIVER_TOP    2   /* first river lane */
#define ROW_RIVER_BOT    6   /* last  river lane */
#define ROW_MEDIAN       7   /* safe median strip */
#define ROW_ROAD_TOP     8   /* first road lane */
#define ROW_ROAD_BOT    12   /* last  road lane */
#define ROW_SAFE        13   /* safe starting strip */

#define FROG_START_ROW  ROW_SAFE
#define FROG_START_COL  39

/* ── Game constants ─────────────────────────────────────────────────────── */
#define HOME_COUNT    5
#define HOME_WIDTH    4     /* columns per home slot */
#define MAX_OBJS     40
#define INIT_LIVES    3
#define DEATH_FRAMES 18
#define LEVEL_CLEAR_FRAMES 50   /* ~4 seconds at 12 ticks/sec */
#define MAX_HIGHSCORES 10

/* Home slot left-edge columns (shared by game.c and render.c) */
extern const int HOME_COLS[HOME_COUNT];

/* ── Object types ───────────────────────────────────────────────────────── */
typedef enum {
    OBJ_CAR,
    OBJ_TRUCK,
    OBJ_LOG,
    OBJ_TURTLE
} ObjKind;

/* ── Moving object ──────────────────────────────────────────────────────── */
typedef struct {
    ObjKind kind;
    int     row;
    float   x;       /* left edge; always kept in [0, GAME_COLS) */
    int     width;
    float   speed;   /* cols/tick – negative means moving left */
    bool    active;
} Obj;

/* ── Frog ───────────────────────────────────────────────────────────────── */
typedef struct {
    int   row, col;
    float frac_x;    /* fractional platform-drift accumulator */
    bool  alive;
    int   death_timer;
} Frog;

typedef struct { bool occupied; } Home;

/* ── High score entry ──────────────────────────────────────────────────── */
typedef struct {
    int  score;
    int  level;
    char name[16];
} HighScore;

/* ── Full game state ────────────────────────────────────────────────────── */
typedef struct {
    Frog  frog;
    Obj   objs[MAX_OBJS];
    Home  homes[HOME_COUNT];
    int   n_objs;
    int   lives;
    int   score;
    int   level;
    int   homes_filled;
    bool  game_over;
    bool  paused;
    int   level_clear_timer;    /* >0 means showing level-complete screen */
} Game;

/* ── game.c API ─────────────────────────────────────────────────────────── */
void game_init(Game *g);
void game_tick(Game *g);
void game_move_frog(Game *g, int drow, int dcol);

/* ── render.c API ───────────────────────────────────────────────────────── */
void render_init(void);
void render_cleanup(void);
void render_frame(const Game *g);
void render_start_screen(const HighScore scores[], int n_scores);
void render_level_clear(const Game *g);

/* ── render.c — name input ─────────────────────────────────────────────── */
void render_name_input(int score, int level, const char *name, int cursor);

/* ── highscore.c API ───────────────────────────────────────────────────── */
int  highscore_load(HighScore scores[], int max);
void highscore_save(const HighScore scores[], int n);
bool highscore_qualifies(const HighScore scores[], int n, int max, int score);
int  highscore_insert(HighScore scores[], int n, int max, int score, int level,
                      const char *name);

#endif /* FROGGER_FROGGER_H */
