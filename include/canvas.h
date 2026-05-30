#ifndef CANVAS_H
#define CANVAS_H

#define ROWS 30
#define COLS 80
#define EMPTY ' '

void canvas_init(void);
void canvas_clear(void);
void canvas_set(int row, int col, char ch);
char canvas_get(int row, int col);
void canvas_display(void);
void canvas_print_raw(void);

#endif
