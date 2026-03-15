#include "frogger.h"
#include <string.h>

/* Home slot left-edge columns (definition – declared extern in frogger.h) */
const int HOME_COLS[HOME_COUNT] = { 4, 17, 30, 43, 56 };

/* ── Lane configuration table ───────────────────────────────────────────── */
typedef struct {
    int     row;
    ObjKind kind;
    float   base_speed; /* positive = right, negative = left */
    int     width;
    int     count;      /* objects per lane */
} LaneDef;

static const LaneDef LANE_DEFS[] = {
    /* River lanes (rows 2-6), top to bottom */
    { ROW_RIVER_TOP + 0, OBJ_TURTLE, -0.25f, 3, 3 },
    { ROW_RIVER_TOP + 1, OBJ_LOG,     0.35f, 7, 2 },
    { ROW_RIVER_TOP + 2, OBJ_LOG,    -0.30f, 5, 3 },
    { ROW_RIVER_TOP + 3, OBJ_TURTLE,  0.20f, 3, 4 },
    { ROW_RIVER_TOP + 4, OBJ_LOG,    -0.40f, 8, 2 },
    /* Road lanes (rows 8-12), top to bottom */
    { ROW_ROAD_TOP + 0, OBJ_CAR,    0.40f, 2, 3 },
    { ROW_ROAD_TOP + 1, OBJ_TRUCK, -0.25f, 4, 2 },
    { ROW_ROAD_TOP + 2, OBJ_CAR,    0.50f, 2, 4 },
    { ROW_ROAD_TOP + 3, OBJ_CAR,   -0.35f, 2, 3 },
    { ROW_ROAD_TOP + 4, OBJ_TRUCK,  0.30f, 4, 2 },
};

#define N_LANES (int)(sizeof(LANE_DEFS) / sizeof(LANE_DEFS[0]))

/* ── Helpers ────────────────────────────────────────────────────────────── */

static bool is_river(int row)
{
    return row >= ROW_RIVER_TOP && row <= ROW_RIVER_BOT;
}

static bool is_road(int row)
{
    return row >= ROW_ROAD_TOP && row <= ROW_ROAD_BOT;
}

/*
 * Returns true if the object occupies column `col`.
 * x is always in [0, GAME_COLS), so an object only wraps when
 * x + width > GAME_COLS.
 */
static bool obj_covers(const Obj *o, int col)
{
    int x = (int)o->x;
    if (x + o->width <= GAME_COLS) {
        return col >= x && col < x + o->width;
    }
    /* Wraps around the right edge */
    return col >= x || col < (x + o->width) % GAME_COLS;
}

/* ── Object spawning ────────────────────────────────────────────────────── */

static void spawn_objects(Game *g)
{
    float boost = 1.0f + (float)(g->level - 1) * 0.15f;
    g->n_objs = 0;

    for (int l = 0; l < N_LANES && g->n_objs < MAX_OBJS; l++) {
        const LaneDef *ld = &LANE_DEFS[l];
        float slot = (float)GAME_COLS / (float)ld->count;

        for (int i = 0; i < ld->count && g->n_objs < MAX_OBJS; i++) {
            Obj *o    = &g->objs[g->n_objs++];
            o->kind   = ld->kind;
            o->row    = ld->row;
            o->speed  = ld->base_speed * boost;
            o->width  = ld->width;
            o->active = true;
            o->x      = slot * (float)i;
        }
    }
}

/* ── Frog life management ───────────────────────────────────────────────── */

static void reset_frog(Game *g)
{
    g->frog.row        = FROG_START_ROW;
    g->frog.col        = FROG_START_COL;
    g->frog.frac_x     = 0.0f;
    g->frog.alive      = true;
    g->frog.death_timer = 0;
}

static void kill_frog(Game *g)
{
    g->lives--;
    g->frog.alive        = false;
    g->frog.death_timer  = DEATH_FRAMES;
    if (g->lives <= 0) {
        g->game_over = true;
    }
}

static void next_level(Game *g)
{
    g->level++;
    g->homes_filled = 0;
    memset(g->homes, 0, sizeof(g->homes));
    spawn_objects(g);
    reset_frog(g);
}

/* ── Public API ─────────────────────────────────────────────────────────── */

void game_init(Game *g)
{
    memset(g, 0, sizeof(*g));
    g->lives = INIT_LIVES;
    g->level = 1;
    spawn_objects(g);
    reset_frog(g);
}

void game_tick(Game *g)
{
    if (g->game_over || g->paused) return;

    /* Count down death animation; objects still move during it */
    if (!g->frog.alive) {
        if (--g->frog.death_timer <= 0 && !g->game_over) {
            reset_frog(g);
        }
    }

    /* Move every object and wrap x into [0, GAME_COLS) */
    for (int i = 0; i < g->n_objs; i++) {
        Obj *o = &g->objs[i];
        if (!o->active) continue;
        o->x += o->speed;
        if (o->x < 0.0f)          o->x += (float)GAME_COLS;
        if (o->x >= (float)GAME_COLS) o->x -= (float)GAME_COLS;
    }

    if (!g->frog.alive) return;

    /* ── River zone: frog must ride a log or turtle ─────────────────── */
    if (is_river(g->frog.row)) {
        const Obj *platform = NULL;
        for (int i = 0; i < g->n_objs; i++) {
            const Obj *o = &g->objs[i];
            if (!o->active) continue;
            if (o->row != g->frog.row) continue;
            if (o->kind != OBJ_LOG && o->kind != OBJ_TURTLE) continue;
            if (obj_covers(o, g->frog.col)) {
                platform = o;
                break;
            }
        }
        if (!platform) {
            kill_frog(g);
            return;
        }
        /* Drift frog with the platform using fractional accumulation */
        g->frog.frac_x += platform->speed;
        int delta = (int)g->frog.frac_x;
        g->frog.frac_x -= (float)delta;
        g->frog.col    += delta;

        if (g->frog.col < 0 || g->frog.col >= GAME_COLS) {
            kill_frog(g);
            return;
        }
    }

    /* ── Road zone: frog must not be hit by a vehicle ───────────────── */
    if (is_road(g->frog.row)) {
        for (int i = 0; i < g->n_objs; i++) {
            const Obj *o = &g->objs[i];
            if (!o->active) continue;
            if (o->row != g->frog.row) continue;
            if (o->kind != OBJ_CAR && o->kind != OBJ_TRUCK) continue;
            if (obj_covers(o, g->frog.col)) {
                kill_frog(g);
                return;
            }
        }
    }

    /* ── Home row: land on an open lily pad ─────────────────────────── */
    if (g->frog.row == ROW_HOME) {
        bool landed = false;
        for (int h = 0; h < HOME_COUNT; h++) {
            if (g->frog.col >= HOME_COLS[h] &&
                g->frog.col <  HOME_COLS[h] + HOME_WIDTH) {
                if (!g->homes[h].occupied) {
                    g->homes[h].occupied = true;
                    g->homes_filled++;
                    g->score += 50 * g->level;
                }
                landed = true;
                reset_frog(g);
                break;
            }
        }
        if (!landed) {
            /* Hit water or an already-occupied pad edge */
            kill_frog(g);
            return;
        }
        if (g->homes_filled >= HOME_COUNT) {
            g->score += 200 * g->level;
            next_level(g);
        }
    }
}

void game_move_frog(Game *g, int drow, int dcol)
{
    if (!g->frog.alive || g->game_over || g->paused) return;

    int new_row = g->frog.row + drow;
    int new_col = g->frog.col + dcol;

    /* Clamp within playfield */
    if (new_row < ROW_HOME) new_row = ROW_HOME;
    if (new_row > ROW_SAFE) new_row = ROW_SAFE;
    if (new_col < 0)        new_col = 0;
    if (new_col >= GAME_COLS) new_col = GAME_COLS - 1;

    /* Award points for moving up */
    if (drow < 0) g->score += 10;

    g->frog.row    = new_row;
    g->frog.col    = new_col;
    g->frog.frac_x = 0.0f; /* reset drift on player input */
}
