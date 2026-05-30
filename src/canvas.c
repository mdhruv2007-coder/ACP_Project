#include "../include/canvas.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

static char canvas[ROWS][COLS];

void canvas_init(void)  { canvas_clear(); }

void canvas_clear(void) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = EMPTY;
}

void canvas_set(int row, int col, char ch) {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
        canvas[row][col] = ch;
}

char canvas_get(int row, int col) {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
        return canvas[row][col];
    return EMPTY;
}

void canvas_display(void) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            mvaddch(r + 1, c + 1, canvas[r][c]);

    mvhline(0,        0, ACS_HLINE, COLS + 2);
    mvhline(ROWS + 1, 0, ACS_HLINE, COLS + 2);
    mvvline(0,        0, ACS_VLINE, ROWS + 2);
    mvvline(0, COLS + 1, ACS_VLINE, ROWS + 2);
    mvaddch(0,        0,        ACS_ULCORNER);
    mvaddch(0,        COLS + 1, ACS_URCORNER);
    mvaddch(ROWS + 1, 0,        ACS_LLCORNER);
    mvaddch(ROWS + 1, COLS + 1, ACS_LRCORNER);
    refresh();
}

void canvas_print_raw(void) {
    for (int c = 0; c < COLS + 2; c++) putchar('-');
    putchar('\n');
    for (int r = 0; r < ROWS; r++) {
        putchar('|');
        for (int c = 0; c < COLS; c++) putchar(canvas[r][c]);
        putchar('|');
        putchar('\n');
    }
    for (int c = 0; c < COLS + 2; c++) putchar('-');
    putchar('\n');
}
