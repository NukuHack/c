/* ============================================================================
 * physics_engine.c -- a tiny, educational 2D physics sandbox for the terminal
 *
 * Build:  cc -O2 -Wall -Wextra -std=c11 physics_engine.c -o physics -lm
 * Run:    ./physics
 *
 * Keys:   q / ESC  quit
 *         SPACE    pause / resume
 *         s        single-step one 1/60 s frame  (auto-pauses)
 *         r        reset the world
 *         + / -    change time scale
 *
 * Layout of this file:
 *   1. small utilities (time, terminal raw mode, terminal size)
 *   2. the data model (Ball / World)
 *   3. world_reset()   -- spawn the initial scene
 *   4. physics_step()  -- the actual "engine": integrate, collide, resolve
 *   5. render()        -- draw the world into a char buffer and blit it
 *   6. main()          -- input handling + a fixed-timestep game loop
 *
 * Every place that would need hardening for production code is marked with a
 * comment like   HARDEN: ...
 * ==========================================================================*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

/* ---------------------------------------------------------------------------
 * Tunables.  All lengths are in "cells" (one terminal character cell), time
 * is in seconds.  NOTE: terminal cells are roughly twice as tall as they are
 * wide, so we compensate in render() by squashing the vertical axis.
 * ------------------------------------------------------------------------ */
#define MAX_BALLS      24
#define MAX_COLS       320          /* HARDEN: fixed caps -> memory is static.  */
#define MAX_ROWS       200          /* A resize-aware version would realloc.    */

#define PHYS_DT        (1.0f / 240.0f)   /* fixed physics sub-step            */
#define FRAME_DT       (1.0f / 60.0f)    /* one "frame" when single-stepping  */

#define GRAVITY        55.0f        /* cells / s^2                            */
#define RESTITUTION    0.88f        /* ball-vs-ball bounciness, 0..1          */
#define WALL_REST      0.85f        /* ball-vs-wall bounciness                */
#define AIR_DRAG       0.985f       /* fraction of velocity kept per second   */

/* ===========================================================================
 * 1. Small utilities
 * ======================================================================== */

static double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static void sleep_ms(long ms)
{
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static struct termios g_saved_termios;
static int            g_termios_ok = 0;

static void term_raw(void)
{
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &g_saved_termios) != 0)
        return;                              /* HARDEN: report the error */

    raw = g_saved_termios;
    raw.c_lflag &= (tcflag_t)~(ECHO | ICANON);   /* no echo, no line buffering */
    raw.c_cc[VMIN]  = 0;                         /* read() returns immediately */
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    g_termios_ok = 1;
}

static void term_restore(void)
{
    if (g_termios_ok)
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_saved_termios);
    g_termios_ok = 0;
}

static void handle_signal(int sig)
{
    (void)sig;
    term_restore();
    fputs("\x1b[?25h\n", stdout);   /* show cursor again */
    /* HARDEN: printf/exit are not async-signal-safe.  For a toy this is fine;
     * a real program would only set a `volatile sig_atomic_t` flag here. */
    exit(0);
}

static void get_terminal_size(int *cols, int *rows)
{
    struct winsize ws;

    *cols = 80;                      /* sane fallback if the query fails */
    *rows = 24;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        if (ws.ws_col > 0) *cols = (int)ws.ws_col;
        if (ws.ws_row > 0) *rows = (int)ws.ws_row;
    }

    /* HARDEN: no SIGWINCH handling -> we never re-read the size after start. */
    if (*cols > MAX_COLS) *cols = MAX_COLS;
    if (*rows > MAX_ROWS) *rows = MAX_ROWS;
    if (*cols < 20)       *cols = 20;
    if (*rows < 8)        *rows = 8;
}

/* ===========================================================================
 * 2. The data model
 * ======================================================================== */

typedef struct {
    float x, y;        /* position of the centre, in cells                  */
    float vx, vy;      /* velocity, cells per second                        */
    float r;           /* radius, in cells                                  */
    float m;           /* mass (> 0!)                                       */
    char  glyph;       /* what to draw it with                              */
} Ball;

typedef struct {
    Ball  balls[MAX_BALLS];
    int   count;
    float width, height;   /* the playfield, in cells                        */
} World;

/* ===========================================================================
 * 3. World construction
 * ======================================================================== */

static void world_reset(World *w, float width, float height)
{
    static const char glyphs[] = "@#%&*+oO0X";
    const int glyph_count = (int)(sizeof glyphs - 1);

    if (!w) return;                                   /* NULL SAFETY */

    w->count  = 0;
    w->width  = (width  > 6.0f) ? width  : 6.0f;      /* guard degenerate sizes */
    w->height = (height > 6.0f) ? height : 6.0f;

    /* Fixed seed so the demo is reproducible -- very handy when you are
     * debugging the solver and want the same initial conditions every run. */
    srand(1337);

    for (int i = 0; i < 14; i++) {
        if (w->count >= MAX_BALLS) break;             /* ARRAY BOUNDS */

        Ball b;
        b.r     = 1.2f + (rand() / (float)RAND_MAX) * 2.6f;   /* 1.2 .. 3.8  */
        b.m     = b.r * b.r;                                  /* mass ~ area */
        b.glyph = glyphs[i % glyph_count];

        /* Keep the spawn box non-negative.  HARDEN: if the terminal is
         * narrower than the ball, this silently produces a tiny box. */
        float max_x = w->width  - 2.0f * b.r - 1.0f;
        float max_y = w->height - 2.0f * b.r - 1.0f;
        if (max_x < 1.0f) max_x = 1.0f;
        if (max_y < 1.0f) max_y = 1.0f;

        /* Rejection sampling: try to find a spot that does not overlap. */
        int placed = 0;
        for (int attempt = 0; attempt < 200 && !placed; attempt++) {
            b.x = b.r + 1.0f + (rand() / (float)RAND_MAX) * max_x;
            b.y = b.r + 1.0f + (rand() / (float)RAND_MAX) * max_y;

            placed = 1;
            for (int j = 0; j < w->count; j++) {
                float dx = b.x - w->balls[j].x;
                float dy = b.y - w->balls[j].y;
                float rs = b.r + w->balls[j].r;
                if (dx * dx + dy * dy < rs * rs) { placed = 0; break; }
            }
        }
        /* If we failed to place it cleanly we just accept the overlap --
         * the positional-correction pass below will push them apart. */

        b.vx = ((rand() / (float)RAND_MAX) - 0.5f) * 40.0f;
        b.vy = ((rand() / (float)RAND_MAX) - 0.5f) * 15.0f;

        w->balls[w->count++] = b;
    }
}

/* ===========================================================================
 * 4. The physics
 * ======================================================================== */

/* One fixed-timestep update.
 *
 * Order matters: integrate -> resolve walls -> resolve pairs.
 * Doing pair resolution last means a ball pushed out of a wall by a
 * neighbour gets corrected on the *next* step, which is fine at 240 Hz.
 */
static void physics_step(World *w, float dt)
{
    if (!w) return;                                    /* NULL SAFETY */
    if (dt <= 0.0f) return;                            /* guard bad dt  */
    if (dt > 0.1f)  dt = 0.1f;                         /* clamp: huge dt
                                                        * means huge
                                                        * tunnelling.  */

    const float drag = powf(AIR_DRAG, dt);             /* frame-rate indep.  */

    /* ---- 4a. Semi-implicit (symplectic) Euler integration ---------------- */
    for (int i = 0; i < w->count; i++) {
        Ball *b = &w->balls[i];

        b->vy += GRAVITY * dt;      /* gravity pulls "down" = +y in terminal  */
        b->vx *= drag;
        b->vy *= drag;

        b->x += b->vx * dt;
        b->y += b->vy * dt;
        /* HARDEN: at high speed a ball can move further than its own radius
         * per step and tunnel straight through a wall or another ball.
         * The classic fixes are continuous collision detection or
         * sub-stepping the motion when |v|*dt > r. */
    }

    /* ---- 4b. Walls ------------------------------------------------------- */
    for (int i = 0; i < w->count; i++) {
        Ball *b = &w->balls[i];

        if (b->x - b->r < 0.0f)          { b->x = b->r;               b->vx = -b->vx * WALL_REST; }
        if (b->x + b->r > w->width)      { b->x = w->width  - b->r;   b->vx = -b->vx * WALL_REST; }
        if (b->y - b->r < 0.0f)          { b->y = b->r;               b->vy = -b->vy * WALL_REST; }
        if (b->y + b->r > w->height)     { b->y = w->height - b->r;   b->vy = -b->vy * WALL_REST; }

        /* HARDEN: if the world is narrower than 2*r the two x-clamps fight
         * each other and the ball jitters.  Clamp the radius on spawn. */
    }

    /* ---- 4c. Ball vs ball ----------------------------------------------- */
    for (int i = 0; i < w->count; i++) {
        for (int j = i + 1; j < w->count; j++) {
            Ball *a = &w->balls[i];
            Ball *b = &w->balls[j];

            float dx   = b->x - a->x;
            float dy   = b->y - a->y;
            float rsum = a->r + b->r;
            float d2   = dx * dx + dy * dy;

            if (d2 >= rsum * rsum) continue;           /* not touching      */

            float d = sqrtf(d2);

            /* Degenerate case: perfectly co-located centres -> the normal is
             * undefined.  Nudge them apart along +x instead of dividing by 0. */
            if (d < 1e-6f) { d = 1e-6f; dx = 1e-6f; dy = 0.0f; }

            float nx = dx / d;                         /* unit normal a -> b */
            float ny = dy / d;

            /* HARDEN: division by zero if any mass is 0.  Invariant: m > 0. */
            float inv_mass_a = 1.0f / a->m;
            float inv_mass_b = 1.0f / b->m;
            float inv_sum    = inv_mass_a + inv_mass_b;

            /* --- positional correction (stops the balls sinking into each
             *     other and jittering forever) ------------------------------ */
            float overlap = rsum - d;
            float corr    = (overlap / inv_sum) * 0.8f;   /* 0.8 = softness  */
            a->x -= corr * nx * inv_mass_a;
            a->y -= corr * ny * inv_mass_a;
            b->x += corr * nx * inv_mass_b;
            b->y += corr * ny * inv_mass_b;

            /* --- impulse along the normal --------------------------------- */
            float rvx = b->vx - a->vx;
            float rvy = b->vy - a->vy;
            float vn  = rvx * nx + rvy * ny;

            if (vn > 0.0f) continue;              /* already separating     */

            float jimp = -(1.0f + RESTITUTION) * vn / inv_sum;

            a->vx -= jimp * nx * inv_mass_a;
            a->vy -= jimp * ny * inv_mass_a;
            b->vx += jimp * nx * inv_mass_b;
            b->vy += jimp * ny * inv_mass_b;

            /* NOTE: this is a purely normal impulse -- no friction, no
             * rotation, no angular momentum.  Good enough for circles that
             * are drawn as blobs of characters. */
        }
    }

    /* HARDEN: no NaN/Inf scrubbing.  A single NaN (e.g. from a bad sqrtf)
     * will silently spread to every ball it touches.  A cheap guard is:
     *     if (!isfinite(b->x) || !isfinite(b->vx)) { reset that ball }
     */
}

/* ===========================================================================
 * 5. Rendering
 * ======================================================================== */

static char canvas[MAX_ROWS][MAX_COLS + 1];   /* +1 for the NUL terminator */

static void render(const World *w, int cols, int rows,
                   int paused, float time_scale, double sim_time)
{
    int world_rows = rows - 1;                 /* last line is the HUD     */
    if (world_rows < 1) world_rows = 1;        /* HARDEN: tiny terminals   */

    /* ---- 5a. clear the char buffer -------------------------------------- */
    for (int y = 0; y < world_rows; y++) {
        memset(canvas[y], ' ', (size_t)cols);
        canvas[y][cols] = '\0';
    }

    /* ---- 5b. rasterise each ball as a filled ellipse ---------------------
     * Terminal cells are ~2x taller than wide, so we multiply dy by 2 in the
     * circle test.  That makes a radius-r ball 2r cells wide but only r cells
     * tall on screen -- which is what makes it look round to a human.      */
    if (w) {
        for (int i = 0; i < w->count; i++) {
            const Ball *b = &w->balls[i];

            int px0 = (int)floorf(b->x - b->r) - 1;
            int px1 = (int)ceilf (b->x + b->r) + 1;
            int py0 = (int)floorf(b->y - b->r * 0.5f) - 1;
            int py1 = (int)ceilf (b->y + b->r * 0.5f) + 1;

            for (int py = py0; py <= py1; py++) {
                if (py < 0 || py >= world_rows) continue;   /* OOB CHECK */
                for (int px = px0; px <= px1; px++) {
                    if (px < 0 || px >= cols) continue;     /* OOB CHECK */

                    float dx = ((float)px + 0.5f) - b->x;
                    float dy = (((float)py + 0.5f) - b->y) * 2.0f;

                    if (dx * dx + dy * dy <= b->r * b->r)
                        canvas[py][px] = b->glyph;
                }
            }
        }
    }

    /* ---- 5c. blit the buffer --------------------------------------------
     * We address each row explicitly with a cursor-position escape instead of
     * relying on '\n'.  Writing the very last column of a terminal sets the
     * "pending wrap" flag and the following newline can then produce a blank
     * line -- explicit positioning sidesteps that entirely.               */
    for (int y = 0; y < world_rows; y++) {
        printf("\x1b[%d;1H", y + 1);
        fwrite(canvas[y], 1, (size_t)cols, stdout);
    }

    /* ---- 5d. HUD --------------------------------------------------------- */
    char hud[256];
    int n = snprintf(hud, sizeof hud,
                     " [space] %-6s  [s] step  [r] reset  [+/-] %.2fx  "
                     "balls %d  t %.2fs ",
                     paused ? "resume" : "pause",
                     (double)time_scale, w ? w->count : 0, sim_time);

    if (n < 0) n = 0;                 /* snprintf error                       */
    if (n > cols) n = cols;           /* HARDEN: truncate rather than wrap    */

    printf("\x1b[%d;1H\x1b[2K", rows);
    fwrite(hud, 1, (size_t)n, stdout);

    fflush(stdout);
}

/* ===========================================================================
 * 6. main -- input + fixed-timestep loop
 * ======================================================================== */

int main(void)
{
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        fprintf(stderr, "physics_engine: needs an interactive terminal.\n");
        return 1;
    }

    int cols, rows;
    get_terminal_size(&cols, &rows);

    int world_rows = rows - 1;
    if (world_rows < 1) world_rows = 1;

    World w;
    world_reset(&w, (float)cols, (float)world_rows);

    term_raw();
    atexit(term_restore);
    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);

    fputs("\x1b[?25l", stdout);   /* hide cursor */
    fputs("\x1b[2J",   stdout);   /* clear screen */

    int    running    = 1;
    int    paused     = 0;
    float  time_scale = 1.0f;
    double accumulator = 0.0;
    double sim_time    = 0.0;
    double last        = now_seconds();

    while (running) {
        /* ---- 6a. drain keyboard input (non-blocking, thanks to raw mode) */
        char c;
        while (read(STDIN_FILENO, &c, 1) == 1) {
            switch (c) {
            case 'q': case 'Q': case 27:              /* 27 = ESC           */
                running = 0;
                break;

            case ' ':
                paused = !paused;
                break;

            case 's': case 'S':
                /* Advance exactly one displayed frame, then stay paused. */
                paused = 1;
                {
                    int subs = (int)(FRAME_DT / PHYS_DT + 0.5f);
                    if (subs < 1) subs = 1;
                    for (int k = 0; k < subs; k++) {
                        physics_step(&w, PHYS_DT);
                        sim_time += PHYS_DT;
                    }
                }
                accumulator = 0.0;
                break;

            case 'r': case 'R':
                world_reset(&w, (float)cols, (float)world_rows);
                sim_time    = 0.0;
                accumulator = 0.0;
                break;

            case '+': case '=':
                time_scale *= 1.5f;
                if (time_scale > 8.0f) time_scale = 8.0f;      /* clamp     */
                break;

            case '-': case '_':
                time_scale /= 1.5f;
                if (time_scale < 0.05f) time_scale = 0.05f;    /* clamp     */
                break;

            default:
                break;
            }
        }

        /* ---- 6b. advance the simulation by a fixed timestep -------------
         * We accumulate real elapsed time and burn it in PHYS_DT chunks so
         * the physics is completely frame-rate independent.               */
        double now   = now_seconds();
        double frame = now - last;
        last = now;

        if (frame > 0.25) frame = 0.25;      /* "spiral of death" clamp      */
        /* HARDEN: without that clamp, a long stall (e.g. the user suspends
         * the process) makes the accumulator huge, and the while-loop below
         * tries to catch up forever, which stalls again -> death spiral.  */

        if (!paused) {
            accumulator += frame * (double)time_scale;

            int guard = 0;
            while (accumulator >= (double)PHYS_DT) {
                physics_step(&w, PHYS_DT);
                sim_time    += PHYS_DT;
                accumulator -= (double)PHYS_DT;

                if (++guard > 1000) {        /* HARDEN: bail out of catch-up */
                    accumulator = 0.0;
                    break;
                }
            }
        }

        /* ---- 6c. draw ---------------------------------------------------- */
        render(&w, cols, rows, paused, time_scale, sim_time);

        /* Crude frame limiter.  HARDEN: measure the time render() took and
         * sleep only the remainder. */
        sleep_ms(8);
    }

    fputs("\x1b[?25h", stdout);   /* show cursor */
    fputs("\x1b[2J\x1b[H", stdout);
    fflush(stdout);
    term_restore();
    return 0;
}