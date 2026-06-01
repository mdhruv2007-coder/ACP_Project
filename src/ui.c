#include "../include/ui.h"
#include "../include/canvas.h"
#include "../include/shapes.h"
#include <ncurses.h>
#include <string.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════════
 * Layout: see canvas.h for STATUS_ROW / PROMPT_ROW / PANEL_COL defines.
 * ═══════════════════════════════════════════════════════════════════════ */

/* ── Helpers ──────────────────────────────────────────────────────────── */

static void clear_prompt_area(int n_rows) {
    for (int i = 0; i < n_rows; i++) {
        move(PROMPT_ROW + i, 0);
        clrtoeol();
    }
}

/* Status bar — always shows live shape count on the right. */
void ui_status(const char *msg) {
    move(STATUS_ROW, 0);
    clrtoeol();
    mvprintw(STATUS_ROW, 1,
             "%-28s [%d/50] | a=add  d=del  m=mod  f=fill  u=undo  c=clear  q=quit",
             msg, get_shape_count());
    refresh();
}

static int prompt_int(int offset, const char *label, int *val) {
    int row = PROMPT_ROW + offset;
    move(row, 0); clrtoeol();
    mvprintw(row, 1, "%s: ", label);
    refresh();
    echo(); curs_set(1);
    int result = scanw("%d", val);
    noecho(); curs_set(0);
    return (result == 1);
}

static int prompt_char(int offset, const char *label, char *out) {
    int row = PROMPT_ROW + offset;
    move(row, 0); clrtoeol();
    mvprintw(row, 1, "%s: ", label);
    refresh();
    echo(); curs_set(1);
    char buf[8] = {0};
    int result = scanw("%7s", buf);
    noecho(); curs_set(0);
    if (result == 1 && buf[0] != '\0') { *out = buf[0]; return 1; }
    return 0;
}

static int check_point(int offset, int r, int c) {
    if (!canvas_in_bounds(r, c)) {
        move(PROMPT_ROW + offset, 0); clrtoeol();
        mvprintw(PROMPT_ROW + offset, 1,
                 "! (%d,%d) out of bounds (rows 0-%d, cols 0-%d)",
                 r, c, CANVAS_ROWS - 1, CANVAS_COLS - 1);
        refresh();
        return 0;
    }
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════
 * ADD SHAPE  (types 1-5, 5 = filled rectangle)
 * ═══════════════════════════════════════════════════════════════════════ */
static void menu_add(void) {
    clear_prompt_area(10);

    mvprintw(PROMPT_ROW, 1,
             "Shape: 1=Line  2=Rectangle  3=Circle  4=Triangle  5=FilledRect  0=cancel");
    refresh();

    int type = 0;
    if (!prompt_int(1, "Enter type", &type) || type == 0) {
        clear_prompt_area(10); ui_status("Cancelled"); return;
    }
    if (type < 1 || type > 5) {
        clear_prompt_area(10); ui_status("! Invalid type — must be 1-5"); return;
    }

    Shape s;
    memset(&s, 0, sizeof(s));
    s.type = type;
    s.ch   = '*';

    switch (type) {

        case SHAPE_LINE: {
            int r1,c1,r2,c2;
            if (!prompt_int(2,"Start row (0-29)",&r1)) goto bad_input;
            if (!prompt_int(3,"Start col (0-79)",&c1)) goto bad_input;
            if (!prompt_int(4,"End   row (0-29)",&r2)) goto bad_input;
            if (!prompt_int(5,"End   col (0-79)",&c2)) goto bad_input;
            if (!check_point(6,r1,c1)||!check_point(6,r2,c2)) goto show_err;
            s.r1=r1; s.c1=c1; s.r2=r2; s.c2=c2;
            break;
        }

        case SHAPE_RECTANGLE: {
            int r,c,h,w;
            if (!prompt_int(2,"Top-left row (0-29)",&r)) goto bad_input;
            if (!prompt_int(3,"Top-left col (0-79)",&c)) goto bad_input;
            if (!prompt_int(4,"Height (>=2)",       &h)) goto bad_input;
            if (!prompt_int(5,"Width  (>=2)",        &w)) goto bad_input;
            if (!check_point(6,r,c)) goto show_err;
            if (h<2||w<2) {
                mvprintw(PROMPT_ROW+6,1,"! Height and width must be >= 2");
                refresh(); goto show_err;
            }
            if (!canvas_in_bounds(r+h-1,c+w-1)) {
                mvprintw(PROMPT_ROW+6,1,"! Rectangle extends off canvas");
                refresh(); goto show_err;
            }
            s.r1=r; s.c1=c; s.r2=h; s.c2=w;
            break;
        }

        case SHAPE_CIRCLE: {
            int cr,cc,rad;
            if (!prompt_int(2,"Centre row (0-29)",&cr))  goto bad_input;
            if (!prompt_int(3,"Centre col (0-79)",&cc))  goto bad_input;
            if (!prompt_int(4,"Radius     (>=1)", &rad)) goto bad_input;
            if (!check_point(5,cr,cc)) goto show_err;
            if (rad<1) {
                mvprintw(PROMPT_ROW+5,1,"! Radius must be >= 1");
                refresh(); goto show_err;
            }
            if (!canvas_in_bounds(cr-rad,cc)||!canvas_in_bounds(cr+rad,cc)||
                !canvas_in_bounds(cr,cc-rad)||!canvas_in_bounds(cr,cc+rad))
                mvprintw(PROMPT_ROW+5,1,
                         "  Note: circle clips canvas edge — clipped pixels skipped");
            refresh();
            s.r1=cr; s.c1=cc; s.radius=rad;
            break;
        }

        case SHAPE_TRIANGLE: {
            int r1,c1,r2,c2,r3,c3;
            if (!prompt_int(2,"P1 row",&r1)) goto bad_input;
            if (!prompt_int(3,"P1 col",&c1)) goto bad_input;
            if (!prompt_int(4,"P2 row",&r2)) goto bad_input;
            if (!prompt_int(5,"P2 col",&c2)) goto bad_input;
            if (!prompt_int(6,"P3 row",&r3)) goto bad_input;
            if (!prompt_int(7,"P3 col",&c3)) goto bad_input;
            if (!check_point(8,r1,c1)) goto show_err;
            if (!check_point(8,r2,c2)) goto show_err;
            if (!check_point(8,r3,c3)) goto show_err;
            s.r1=r1;s.c1=c1;s.r2=r2;s.c2=c2;s.r3=r3;s.c3=c3;
            break;
        }

        case SHAPE_RECT_FILLED: {
            int r,c,h,w; char fch='#';
            if (!prompt_int(2,"Top-left row (0-29)",&r))  goto bad_input;
            if (!prompt_int(3,"Top-left col (0-79)",&c))  goto bad_input;
            if (!prompt_int(4,"Height (>=3)",        &h)) goto bad_input;
            if (!prompt_int(5,"Width  (>=3)",        &w)) goto bad_input;
            if (!prompt_char(6,"Fill char (e.g. # . + @)",&fch)) goto bad_input;
            if (!check_point(7,r,c)) goto show_err;
            if (h<3||w<3) {
                mvprintw(PROMPT_ROW+7,1,
                         "! Filled rect needs height>=3 and width>=3 (needs interior)");
                refresh(); goto show_err;
            }
            if (!canvas_in_bounds(r+h-1,c+w-1)) {
                mvprintw(PROMPT_ROW+7,1,"! Rectangle extends off canvas");
                refresh(); goto show_err;
            }
            s.r1=r; s.c1=c; s.r2=h; s.c2=w; s.ch=fch;
            break;
        }
    }

    {
        int id = add_shape(s);
        clear_prompt_area(10);
        if (id < 0) ui_status("! Canvas full — delete a shape first");
        else {
            char msg[48];
            snprintf(msg,sizeof(msg),"Added shape #%d",id);
            ui_status(msg);
        }
        panel_draw_shapes();
        return;
    }

bad_input:
    mvprintw(PROMPT_ROW+8,1,"! Non-numeric input — cancelled");
    refresh();
show_err:
    mvprintw(PROMPT_ROW+9,1,"  Press any key to continue...");
    refresh(); getch();
    clear_prompt_area(10);
    ui_status("Cancelled");
    panel_draw_shapes();
}

/* ═══════════════════════════════════════════════════════════════════════
 * FILL RECTANGLE  (f key)
 * Asks for an existing rectangle shape ID + a fill character,
 * converts it to a SHAPE_RECT_FILLED in-place.
 * ═══════════════════════════════════════════════════════════════════════ */
static void menu_fill(void) {
    clear_prompt_area(6);

    if (get_shape_count() == 0) {
        mvprintw(PROMPT_ROW,1,"  Nothing to fill — canvas is empty.");
        mvprintw(PROMPT_ROW+1,1,"  Press any key..."); refresh(); getch();
        clear_prompt_area(6); ui_status("Nothing to fill"); return;
    }

    int id = 0;
    if (!prompt_int(0,"Fill rectangle ID (must be a Rect)",&id)) {
        clear_prompt_area(6); ui_status("! Non-numeric — cancelled"); return;
    }

    Shape *s = find_shape(id);
    if (!s) {
        mvprintw(PROMPT_ROW+1,1,"! Shape #%d not found. Press any key...",id);
        refresh(); getch();
        clear_prompt_area(6); ui_status("! ID not found"); return;
    }
    if (s->type != SHAPE_RECTANGLE) {
        mvprintw(PROMPT_ROW+1,1,
                 "! Shape #%d is not a plain Rectangle (type=%d). Press any key...",
                 id, s->type);
        refresh(); getch();
        clear_prompt_area(6); ui_status("! Only plain rectangles can be filled"); return;
    }
    if (s->r2 < 3 || s->c2 < 3) {
        mvprintw(PROMPT_ROW+1,1,
                 "! Rectangle #%d is too small to fill (needs h>=3, w>=3). Press any key...",
                 id);
        refresh(); getch();
        clear_prompt_area(6); ui_status("! Rectangle too small to fill"); return;
    }

    char fch = '#';
    if (!prompt_char(1,"Fill char (e.g. # . + @)",&fch)) {
        clear_prompt_area(6); ui_status("! Bad input — cancelled"); return;
    }

    /* Convert in-place: change type + fill char, keep all coords */
    Shape updated  = *s;
    updated.type   = SHAPE_RECT_FILLED;
    updated.ch     = fch;
    modify_shape(id, updated);

    clear_prompt_area(6);
    char msg[48];
    snprintf(msg,sizeof(msg),"Filled rect #%d with '%c'",id,fch);
    ui_status(msg);
    panel_draw_shapes();
}

/* ═══════════════════════════════════════════════════════════════════════
 * DELETE SHAPE
 * ═══════════════════════════════════════════════════════════════════════ */
static void menu_delete(void) {
    clear_prompt_area(4);

    if (get_shape_count() == 0) {
        mvprintw(PROMPT_ROW,1,"  Nothing to delete — canvas is empty.");
        mvprintw(PROMPT_ROW+1,1,"  Press any key..."); refresh(); getch();
        clear_prompt_area(4); ui_status("Nothing to delete"); return;
    }

    int id = 0;
    if (!prompt_int(0,"Delete shape ID (see panel)",&id)) {
        clear_prompt_area(4); ui_status("! Non-numeric — cancelled"); return;
    }
    if (id <= 0) { clear_prompt_area(4); ui_status("! Invalid ID"); return; }

    if (delete_shape(id) == 0) {
        clear_prompt_area(4);
        char msg[48];
        snprintf(msg,sizeof(msg),"Deleted #%d  (u=undo)",id);
        ui_status(msg);
    } else {
        mvprintw(PROMPT_ROW+1,1,"! Shape #%d not found. Press any key...",id);
        refresh(); getch();
        clear_prompt_area(4); ui_status("! ID not found — no change");
    }
    panel_draw_shapes();
}

/* ═══════════════════════════════════════════════════════════════════════
 * UNDO  (u key) — restores last deleted shape
 * ═══════════════════════════════════════════════════════════════════════ */
static void menu_undo(void) {
    clear_prompt_area(2);
    int new_id = undo_last_delete();
    if (new_id < 0) {
        mvprintw(PROMPT_ROW,1,"  Nothing to undo. Press any key...");
        refresh(); getch();
        clear_prompt_area(2); ui_status("Nothing to undo");
    } else {
        clear_prompt_area(2);
        char msg[48];
        snprintf(msg,sizeof(msg),"Undo: shape restored as #%d",new_id);
        ui_status(msg);
    }
    panel_draw_shapes();
}

/* ═══════════════════════════════════════════════════════════════════════
 * MODIFY SHAPE
 * ═══════════════════════════════════════════════════════════════════════ */
static void menu_modify(void) {
    clear_prompt_area(10);

    if (get_shape_count() == 0) {
        mvprintw(PROMPT_ROW,1,"  Nothing to modify — canvas is empty.");
        mvprintw(PROMPT_ROW+1,1,"  Press any key..."); refresh(); getch();
        clear_prompt_area(4); ui_status("Nothing to modify"); return;
    }

    int id = 0;
    if (!prompt_int(0,"Modify shape ID (see panel)",&id)) {
        clear_prompt_area(10); ui_status("! Non-numeric — cancelled"); return;
    }

    Shape *existing = find_shape(id);
    if (!existing) {
        mvprintw(PROMPT_ROW+1,1,"! Shape #%d not found. Press any key...",id);
        refresh(); getch();
        clear_prompt_area(10); ui_status("! ID not found — no change"); return;
    }

    Shape updated = *existing;

    switch (updated.type) {
        case SHAPE_LINE: {
            int r1,c1,r2,c2;
            if (!prompt_int(1,"New start row",&r1)) goto bad_mod;
            if (!prompt_int(2,"New start col",&c1)) goto bad_mod;
            if (!prompt_int(3,"New end   row",&r2)) goto bad_mod;
            if (!prompt_int(4,"New end   col",&c2)) goto bad_mod;
            if (!check_point(5,r1,c1)||!check_point(5,r2,c2)) goto show_mod_err;
            updated.r1=r1;updated.c1=c1;updated.r2=r2;updated.c2=c2;
            break;
        }
        case SHAPE_RECTANGLE:
        case SHAPE_RECT_FILLED: {
            int r,c,h,w; char fch = updated.ch;
            if (!prompt_int(1,"New top-left row",&r)) goto bad_mod;
            if (!prompt_int(2,"New top-left col",&c)) goto bad_mod;
            if (!prompt_int(3,"New height (>=2)",&h)) goto bad_mod;
            if (!prompt_int(4,"New width  (>=2)",&w)) goto bad_mod;
            if (updated.type == SHAPE_RECT_FILLED)
                if (!prompt_char(5,"New fill char",&fch)) goto bad_mod;
            if (!check_point(5,r,c)) goto show_mod_err;
            int min_dim = (updated.type == SHAPE_RECT_FILLED) ? 3 : 2;
            if (h < min_dim || w < min_dim) {
                mvprintw(PROMPT_ROW+5,1,"! Dimension too small"); refresh();
                goto show_mod_err;
            }
            if (!canvas_in_bounds(r+h-1,c+w-1)) {
                mvprintw(PROMPT_ROW+5,1,"! Rectangle extends off canvas");
                refresh(); goto show_mod_err;
            }
            updated.r1=r;updated.c1=c;updated.r2=h;updated.c2=w;updated.ch=fch;
            break;
        }
        case SHAPE_CIRCLE: {
            int cr,cc,rad;
            if (!prompt_int(1,"New centre row",&cr))   goto bad_mod;
            if (!prompt_int(2,"New centre col",&cc))   goto bad_mod;
            if (!prompt_int(3,"New radius (>=1)",&rad)) goto bad_mod;
            if (!check_point(4,cr,cc)) goto show_mod_err;
            if (rad<1) {
                mvprintw(PROMPT_ROW+4,1,"! Radius must be >= 1");
                refresh(); goto show_mod_err;
            }
            updated.r1=cr;updated.c1=cc;updated.radius=rad;
            break;
        }
        case SHAPE_TRIANGLE: {
            int r1,c1,r2,c2,r3,c3;
            if (!prompt_int(1,"New P1 row",&r1)) goto bad_mod;
            if (!prompt_int(2,"New P1 col",&c1)) goto bad_mod;
            if (!prompt_int(3,"New P2 row",&r2)) goto bad_mod;
            if (!prompt_int(4,"New P2 col",&c2)) goto bad_mod;
            if (!prompt_int(5,"New P3 row",&r3)) goto bad_mod;
            if (!prompt_int(6,"New P3 col",&c3)) goto bad_mod;
            if (!check_point(7,r1,c1)) goto show_mod_err;
            if (!check_point(7,r2,c2)) goto show_mod_err;
            if (!check_point(7,r3,c3)) goto show_mod_err;
            updated.r1=r1;updated.c1=c1;
            updated.r2=r2;updated.c2=c2;
            updated.r3=r3;updated.c3=c3;
            break;
        }
    }

    modify_shape(id, updated);
    {
        clear_prompt_area(10);
        char msg[48];
        snprintf(msg,sizeof(msg),"Modified shape #%d",id);
        ui_status(msg);
        panel_draw_shapes();
        return;
    }

bad_mod:
    mvprintw(PROMPT_ROW+8,1,"! Non-numeric input — shape unchanged");
    refresh();
show_mod_err:
    mvprintw(PROMPT_ROW+9,1,"  Press any key to continue...");
    refresh(); getch();
    clear_prompt_area(10);
    ui_status("Cancelled — shape unchanged");
    panel_draw_shapes();
}

/* ═══════════════════════════════════════════════════════════════════════
 * CLEAR CANVAS
 * ═══════════════════════════════════════════════════════════════════════ */
static void menu_clear(void) {
    clear_prompt_area(3);

    if (get_shape_count() == 0) {
        ui_status("Canvas already empty"); return;
    }

    mvprintw(PROMPT_ROW, 1,
             "Clear canvas and delete ALL %d shape(s)?  y=yes  n=no",
             get_shape_count());
    refresh();
    int ch = getch();
    clear_prompt_area(3);

    if (ch == 'y' || ch == 'Y') {
        shapes_init();
        canvas_clear();
        canvas_display();
        panel_draw_shapes();
        ui_status("Canvas cleared");
    } else {
        ui_status("Clear cancelled");
        panel_draw_shapes();
    }
}

/* ═══════════════════════════════════════════════════════════════════════
 * MAIN EVENT LOOP
 * ═══════════════════════════════════════════════════════════════════════ */
void ui_run(void) {
    panel_draw_shapes();
    ui_status("Ready");

    int ch;
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case 'a': menu_add();    break;
            case 'd': menu_delete(); break;
            case 'm': menu_modify(); break;
            case 'f': menu_fill();   break;
            case 'u': menu_undo();   break;
            case 'c': menu_clear();  break;
            case 'l':
                clear_prompt_area(2);
                panel_draw_shapes();
                ui_status("Ready");
                break;
            default: break;
        }
    }
}
