#include "../include/ui.h"
#include "../include/canvas.h"
#include "../include/shapes.h"
#include <ncurses.h>
#include <stdio.h>

void ui_status(const char *msg) {
    move(ROWS + 2, 0);
    clrtoeol();
    mvprintw(ROWS + 2, 1, "[ %s ]  q=quit  c=clear", msg);
    refresh();
}

void ui_run(void) {
    int ch;
    ui_status("Ready");
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case 'c':
                canvas_clear();
                canvas_display();
                ui_status("Canvas cleared");
                break;
            /* Phase 4: add 'a', 'd', 'm' menu handlers here */
            default:
                break;
        }
    }
}
