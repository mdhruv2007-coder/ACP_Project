#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/canvas.h"
#include "../include/shapes.h"
#include "../include/ui.h"

int main(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    canvas_init();

    /* Demo shapes */
    draw_line(2, 2, 2, 30, '_');
    draw_rectangle(5, 5, 8, 20);
    draw_circle(14, 55, 7);
    draw_triangle(22, 30, 27, 20, 27, 40);

    canvas_display();
    ui_run();

    endwin();
    return EXIT_SUCCESS;
}
