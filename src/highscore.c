#include "frogger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* ── Data directory ────────────────────────────────────────────────────── */

static void get_data_dir(char *buf, size_t buflen)
{
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";

    /* Use XDG_DATA_HOME if set, otherwise ~/.local/share */
    const char *xdg = getenv("XDG_DATA_HOME");
    if (xdg && xdg[0] != '\0') {
        snprintf(buf, buflen, "%s/byte_hopper", xdg);
    } else {
        snprintf(buf, buflen, "%s/.local/share/byte_hopper", home);
    }
}

static void ensure_dir(const char *path)
{
    /* Create parent (~/.local/share) if needed, then the target */
    char parent[512];
    snprintf(parent, sizeof(parent), "%s", path);
    char *last_slash = strrchr(parent, '/');
    if (last_slash) {
        *last_slash = '\0';
        mkdir(parent, 0755);
    }
    mkdir(path, 0755);
}

static void get_score_path(char *buf, size_t buflen)
{
    char dir[512];
    get_data_dir(dir, sizeof(dir));
    ensure_dir(dir);
    snprintf(buf, buflen, "%s/highscores.dat", dir);
}

/* ── Load / Save ───────────────────────────────────────────────────────── */

int highscore_load(HighScore scores[], int max)
{
    char path[600];
    get_score_path(path, sizeof(path));

    FILE *f = fopen(path, "r");
    if (!f) return 0;

    int n = 0;
    while (n < max) {
        char line[128];
        if (!fgets(line, (int)sizeof(line), f)) break;

        HighScore hs = {0};
        /* Format: score level name */
        if (sscanf(line, "%d %d %15s", &hs.score, &hs.level, hs.name) >= 2) {
            if (hs.name[0] == '\0') {
                snprintf(hs.name, sizeof(hs.name), "---");
            }
            scores[n++] = hs;
        }
    }
    fclose(f);
    return n;
}

void highscore_save(const HighScore scores[], int n)
{
    char path[600];
    get_score_path(path, sizeof(path));

    FILE *f = fopen(path, "w");
    if (!f) return;

    for (int i = 0; i < n; i++) {
        fprintf(f, "%d %d %s\n", scores[i].score, scores[i].level, scores[i].name);
    }
    fclose(f);
}

/* Returns true if the score would make it onto the high score list. */
bool highscore_qualifies(const HighScore scores[], int n, int max, int score)
{
    if (score <= 0) return false;
    if (n < max) return true;
    return score > scores[n - 1].score;
}

/* Insert a new score in sorted (descending) order. Returns new count. */
int highscore_insert(HighScore scores[], int n, int max, int score, int level,
                     const char *name)
{
    /* Find insertion point */
    int pos = n;
    for (int i = 0; i < n; i++) {
        if (score > scores[i].score) {
            pos = i;
            break;
        }
    }

    if (pos >= max) return n; /* didn't make the list */

    /* Shift entries down */
    int new_n = n < max ? n + 1 : max;
    for (int i = new_n - 1; i > pos; i--) {
        scores[i] = scores[i - 1];
    }

    scores[pos].score = score;
    scores[pos].level = level;
    snprintf(scores[pos].name, sizeof(scores[pos].name), "%s",
             (name && name[0] != '\0') ? name : "???");

    return new_n;
}
