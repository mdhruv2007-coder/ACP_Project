#include "../include/shapes.h"
#include "../include/canvas.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

/* ── Shape store ─────────────────────────────────────────────────────── */
static Shape shapes[MAX_SHAPES];
static int   shape_count = 0;
static int   next_id     = 1;

/* ─────────────────────────────────────────────────────────────────────
 * Drawing primitives
 * ───────────────────────────────────────────────────────────────────── */
static int iabs(int x) { return x < 0 ? -x : x; }

void draw_line(int r1, int c1, int r2, int c2, char ch) {
    int dr = iabs(r2-r1), dc = iabs(c2-c1);
    int sr = (r1 < r2) ? 1 : -1, sc = (c1 < c2) ? 1 : -1;
    int err = dr - dc;
    while (1) {
        canvas_set(r1, c1, ch);
        if (r1 == r2 && c1 == c2) break;
        int e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r1 += sr; }
        if (e2 <  dr) { err += dr; c1 += sc; }
    }
}

void draw_rectangle(int r, int c, int h, int w) {
    for (int col = c; col < c + w; col++) {
        canvas_set(r,         col, '_');
        canvas_set(r + h - 1, col, '_');
    }
    for (int row = r + 1; row < r + h - 1; row++) {
        canvas_set(row, c,         '*');
        canvas_set(row, c + w - 1, '*');
    }
    canvas_set(r,         c,         '*');
    canvas_set(r,         c + w - 1, '*');
    canvas_set(r + h - 1, c,         '*');
    canvas_set(r + h - 1, c + w - 1, '*');
}

void draw_circle(int cr, int cc, int radius) {
    int x = 0, y = radius, p = 1 - radius;
    while (x <= y) {
        canvas_set(cr+y, cc+x, '*'); canvas_set(cr-y, cc+x, '*');
        canvas_set(cr+y, cc-x, '*'); canvas_set(cr-y, cc-x, '*');
        canvas_set(cr+x, cc+y, '*'); canvas_set(cr-x, cc+y, '*');
        canvas_set(cr+x, cc-y, '*'); canvas_set(cr-x, cc-y, '*');
        x++;
        if (p < 0) p += 2*x + 1;
        else { y--; p += 2*(x-y) + 1; }
    }
}

void draw_triangle(int r1, int c1, int r2, int c2, int r3, int c3) {
    draw_line(r1, c1, r2, c2, '*');
    draw_line(r2, c2, r3, c3, '*');
    draw_line(r3, c3, r1, c1, '*');
}

/* ─────────────────────────────────────────────────────────────────────
 * Internal: render one shape onto the canvas
 * ───────────────────────────────────────────────────────────────────── */
static void render_shape(const Shape *s) {
    switch (s->type) {
        case SHAPE_LINE:
            draw_line(s->r1, s->c1, s->r2, s->c2, s->ch);
            break;
        case SHAPE_RECTANGLE:
            draw_rectangle(s->r1, s->c1, s->r2, s->c2);
            break;
        case SHAPE_CIRCLE:
            draw_circle(s->r1, s->c1, s->radius);
            break;
        case SHAPE_TRIANGLE:
            draw_triangle(s->r1, s->c1, s->r2, s->c2, s->r3, s->c3);
            break;
    }
}

/* ─────────────────────────────────────────────────────────────────────
 * shapes_init — clear the shape store
 * ───────────────────────────────────────────────────────────────────── */
void shapes_init(void) {
    for (int i = 0; i < MAX_SHAPES; i++)
        shapes[i].active = 0;
    shape_count = 0;
    next_id     = 1;
}

/* ─────────────────────────────────────────────────────────────────────
 * add_shape — store a shape, assign it an id, render it
 * ───────────────────────────────────────────────────────────────────── */
int add_shape(Shape s) {
    if (shape_count >= MAX_SHAPES) return -1;
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (!shapes[i].active) {
            s.id     = next_id++;
            s.active = 1;
            shapes[i] = s;
            shape_count++;
            render_shape(&shapes[i]);
            canvas_display();
            return s.id;
        }
    }
    return -1;
}

/* ─────────────────────────────────────────────────────────────────────
 * delete_shape — mark as inactive, redraw everything
 * ───────────────────────────────────────────────────────────────────── */
int delete_shape(int id) {
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active && shapes[i].id == id) {
            shapes[i].active = 0;
            shape_count--;
            redraw_all();
            return 0;
        }
    }
    return -1;
}

/* ─────────────────────────────────────────────────────────────────────
 * find_shape — return pointer to shape with given id, or NULL
 * ───────────────────────────────────────────────────────────────────── */
Shape *find_shape(int id) {
    for (int i = 0; i < MAX_SHAPES; i++)
        if (shapes[i].active && shapes[i].id == id)
            return &shapes[i];
    return NULL;
}

/* ─────────────────────────────────────────────────────────────────────
 * modify_shape — replace fields, keep same id, redraw
 * ───────────────────────────────────────────────────────────────────── */
int modify_shape(int id, Shape updated) {
    Shape *s = find_shape(id);
    if (!s) return -1;
    updated.id     = id;
    updated.active = 1;
    *s = updated;
    redraw_all();
    return 0;
}

/* ─────────────────────────────────────────────────────────────────────
 * redraw_all — wipe canvas, re-render every active shape
 * ───────────────────────────────────────────────────────────────────── */
void redraw_all(void) {
    canvas_clear();
    for (int i = 0; i < MAX_SHAPES; i++)
        if (shapes[i].active)
            render_shape(&shapes[i]);
    canvas_display();
}

/* ─────────────────────────────────────────────────────────────────────
 * panel_draw_shapes — render shape list in the right-side panel.
 *
 * Layout (PANEL_COL is defined in canvas.h):
 *
 *   col PANEL_COL
 *   ┌─ Shapes (n/50) ──────┐
 *   │ #1  Line   r1,c1     │
 *   │ #2  Rect   r1,c1     │
 *   │  ...                 │
 *   └──────────────────────┘
 *
 * The panel occupies rows 0 .. CANVAS_ROWS+1 (same height as canvas).
 * ───────────────────────────────────────────────────────────────────── */
void panel_draw_shapes(void) {
    const char *names[] = {"", "Line", "Rect", "Circ", "Tri "};
    int max_rows = CANVAS_ROWS;   /* how many shape rows fit in panel */

    /* Panel border */
    mvhline(0,             PANEL_COL - 1, ACS_HLINE, PANEL_W + 2);
    mvhline(CANVAS_ROWS+1, PANEL_COL - 1, ACS_HLINE, PANEL_W + 2);
    mvvline(0,             PANEL_COL - 1, ACS_VLINE, CANVAS_ROWS + 2);
    mvvline(0,             PANEL_COL + PANEL_W, ACS_VLINE, CANVAS_ROWS + 2);
    mvaddch(0,             PANEL_COL - 1,         ACS_ULCORNER);
    mvaddch(0,             PANEL_COL + PANEL_W,   ACS_URCORNER);
    mvaddch(CANVAS_ROWS+1, PANEL_COL - 1,         ACS_LLCORNER);
    mvaddch(CANVAS_ROWS+1, PANEL_COL + PANEL_W,   ACS_LRCORNER);

    /* Header */
    mvprintw(0, PANEL_COL, " Shapes %d/%-2d ", shape_count, MAX_SHAPES);

    /* Clear panel interior */
    for (int r = 1; r <= CANVAS_ROWS; r++) {
        move(r, PANEL_COL);
        for (int c = 0; c < PANEL_W; c++) addch(' ');
    }

    if (shape_count == 0) {
        mvprintw(1, PANEL_COL, " (empty canvas)");
        refresh();
        return;
    }

    /* Column headers */
    mvprintw(1, PANEL_COL, " ID  Type  Coords");

    int display_row = 2;
    for (int i = 0; i < MAX_SHAPES && display_row <= max_rows; i++) {
        if (!shapes[i].active) continue;
        const Shape *s = &shapes[i];
        const char  *nm = (s->type >= 1 && s->type <= 4) ? names[s->type] : "????";

        switch (s->type) {
            case SHAPE_LINE:
                mvprintw(display_row, PANEL_COL,
                         " #%-2d %s  (%d,%d)->(%d,%d)",
                         s->id, nm, s->r1, s->c1, s->r2, s->c2);
                break;
            case SHAPE_RECTANGLE:
                mvprintw(display_row, PANEL_COL,
                         " #%-2d %s  (%d,%d) %dx%d",
                         s->id, nm, s->r1, s->c1, s->r2, s->c2);
                break;
            case SHAPE_CIRCLE:
                mvprintw(display_row, PANEL_COL,
                         " #%-2d %s  ctr(%d,%d) r=%d",
                         s->id, nm, s->r1, s->c1, s->radius);
                break;
            case SHAPE_TRIANGLE:
                mvprintw(display_row, PANEL_COL,
                         " #%-2d %s  (%d,%d)(%d,%d)(%d,%d)",
                         s->id, nm,
                         s->r1, s->c1, s->r2, s->c2, s->r3, s->c3);
                break;
        }
        display_row++;
    }

    /* Scroll hint if more shapes than panel rows */
    if (shape_count > max_rows - 2) {
        mvprintw(CANVAS_ROWS, PANEL_COL, " ... use 'l' to list all");
    }

    refresh();
}

/* ─────────────────────────────────────────────────────────────────────
 * list_shapes — kept for backwards-compat; delegates to panel
 * ───────────────────────────────────────────────────────────────────── */
void list_shapes(void) {
    panel_draw_shapes();
}

int get_shape_count(void) { return shape_count; }
