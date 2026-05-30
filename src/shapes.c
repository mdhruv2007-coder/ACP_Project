#include "../include/canvas.h"
#include "../include/shapes.h"

static int iabs(int x) { return x < 0 ? -x : x; }

void draw_line(int r1, int c1, int r2, int c2, char ch) {
    int dr = iabs(r2 - r1), dc = iabs(c2 - c1);
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
        canvas_set(cr + y, cc + x, '*'); canvas_set(cr - y, cc + x, '*');
        canvas_set(cr + y, cc - x, '*'); canvas_set(cr - y, cc - x, '*');
        canvas_set(cr + x, cc + y, '*'); canvas_set(cr - x, cc + y, '*');
        canvas_set(cr + x, cc - y, '*'); canvas_set(cr - x, cc - y, '*');
        x++;
        if (p < 0) p += 2 * x + 1;
        else { y--; p += 2 * (x - y) + 1; }
    }
}

void draw_triangle(int r1, int c1, int r2, int c2, int r3, int c3) {
    draw_line(r1, c1, r2, c2, '*');
    draw_line(r2, c2, r3, c3, '*');
    draw_line(r3, c3, r1, c1, '*');
}
