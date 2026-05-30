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
    shapes_init();
    canvas_display();
    ui_run();

    endwin();
    return EXIT_SUCCESS;
}
