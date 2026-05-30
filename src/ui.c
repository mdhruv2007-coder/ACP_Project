#include "../include/ui.h"
#include "../include/canvas.h"
#include "../include/shapes.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

/* ── Status bar ──────────────────────────────────────────────────────── */
void ui_status(const char *msg) {
    move(CANVAS_ROWS + 2, 0);
    clrtoeol();
    mvprintw(CANVAS_ROWS + 2, 1,
             "[ %s ]  a=add  d=delete  m=modify  c=clear  l=list  q=quit", msg);
    refresh();
}

/* ── Prompt helpers ──────────────────────────────────────────────────── */
static void prompt(int row, const char *label, int *val) {
    move(row, 0); clrtoeol();
    mvprintw(row, 1, "%s: ", label);
    echo(); curs_set(1);
    scanw("%d", val);
    noecho(); curs_set(0);
}

/* ── Add shape menu ──────────────────────────────────────────────────── */
static void menu_add(void) {
    int base = CANVAS_ROWS + 3;
    move(base, 0); clrtoeol();
    mvprintw(base, 1, "Shape: 1=Line  2=Rectangle  3=Circle  4=Triangle");
    refresh();

    echo(); curs_set(1);
    int type = 0;
    scanw("%d", &type);
    noecho(); curs_set(0);

    if (type < 1 || type > 4) {
        ui_status("Invalid type — cancelled");
        return;
    }

    Shape s;
    memset(&s, 0, sizeof(s));
    s.type = type;
    s.ch   = '*';

    switch (type) {
        case SHAPE_LINE:
            prompt(base+1, "Start row", &s.r1);
            prompt(base+2, "Start col", &s.c1);
            prompt(base+3, "End row",   &s.r2);
            prompt(base+4, "End col",   &s.c2);
            break;
        case SHAPE_RECTANGLE:
            prompt(base+1, "Top-left row",  &s.r1);
            prompt(base+2, "Top-left col",  &s.c1);
            prompt(base+3, "Height",        &s.r2);  /* reuse r2 for h */
            prompt(base+4, "Width",         &s.c2);  /* reuse c2 for w */
            break;
        case SHAPE_CIRCLE:
            prompt(base+1, "Centre row", &s.r1);
            prompt(base+2, "Centre col", &s.c1);
            prompt(base+3, "Radius",     &s.radius);
            break;
        case SHAPE_TRIANGLE:
            prompt(base+1, "Point1 row", &s.r1);
            prompt(base+2, "Point1 col", &s.c1);
            prompt(base+3, "Point2 row", &s.r2);
            prompt(base+4, "Point2 col", &s.c2);
            prompt(base+5, "Point3 row", &s.r3);
            prompt(base+6, "Point3 col", &s.c3);
            break;
    }

    int id = add_shape(s);
    if (id < 0)
        ui_status("Canvas full — delete a shape first");
    else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Added shape id=%d", id);
        ui_status(msg);
    }
    list_shapes();
}

/* ── Delete shape menu ───────────────────────────────────────────────── */
static void menu_delete(void) {
    int base = CANVAS_ROWS + 3;
    list_shapes();
    int id = 0;
    prompt(base + 1, "Delete shape id", &id);
    if (delete_shape(id) == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Deleted shape id=%d", id);
        ui_status(msg);
    } else {
        ui_status("ID not found");
    }
    list_shapes();
}

/* ── Modify shape menu ───────────────────────────────────────────────── */
static void menu_modify(void) {
    int base = CANVAS_ROWS + 3;
    list_shapes();
    int id = 0;
    prompt(base + 1, "Modify shape id", &id);

    Shape *existing = find_shape(id);
    if (!existing) { ui_status("ID not found"); return; }

    /* Start from a copy so type is preserved */
    Shape updated = *existing;

    switch (updated.type) {
        case SHAPE_LINE:
            prompt(base+2, "New start row", &updated.r1);
            prompt(base+3, "New start col", &updated.c1);
            prompt(base+4, "New end row",   &updated.r2);
            prompt(base+5, "New end col",   &updated.c2);
            break;
        case SHAPE_RECTANGLE:
            prompt(base+2, "New top-left row", &updated.r1);
            prompt(base+3, "New top-left col", &updated.c1);
            prompt(base+4, "New height",       &updated.r2);
            prompt(base+5, "New width",        &updated.c2);
            break;
        case SHAPE_CIRCLE:
            prompt(base+2, "New centre row", &updated.r1);
            prompt(base+3, "New centre col", &updated.c1);
            prompt(base+4, "New radius",     &updated.radius);
            break;
        case SHAPE_TRIANGLE:
            prompt(base+2, "New P1 row", &updated.r1);
            prompt(base+3, "New P1 col", &updated.c1);
            prompt(base+4, "New P2 row", &updated.r2);
            prompt(base+5, "New P2 col", &updated.c2);
            prompt(base+6, "New P3 row", &updated.r3);
            prompt(base+7, "New P3 col", &updated.c3);
            break;
    }

    modify_shape(id, updated);
    char msg[64];
    snprintf(msg, sizeof(msg), "Modified shape id=%d", id);
    ui_status(msg);
    list_shapes();
}

/* ── Main event loop ─────────────────────────────────────────────────── */
void ui_run(void) {
    ui_status("Ready");
    list_shapes();
    int ch;
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case 'a': menu_add();    break;
            case 'd': menu_delete(); break;
            case 'm': menu_modify(); break;
            case 'c':
                canvas_clear();
                canvas_display();
                ui_status("Canvas cleared");
                list_shapes();
                break;
            case 'l':
                list_shapes();
                ui_status("Ready");
                break;
            default:
                break;
        }
    }
}
