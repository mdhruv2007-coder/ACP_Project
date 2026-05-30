#ifndef CANVAS_H
#define CANVAS_H

#define CANVAS_ROWS 30
#define CANVAS_COLS 80
#define EMPTY ' '

/* ── Screen layout constants ─────────────────────────────────────────
 *
 *   col 0                col 81   col 83
 *   ┌──────────────────────┐      ┌─────────────────────┐
 *   │  canvas  (80 wide)   │      │  shape panel        │
 *   │  CANVAS_ROWS tall    │      │  (PANEL_W wide)     │
 *   └──────────────────────┘      └─────────────────────┘
 *   row CANVAS_ROWS+2  → status bar
 *   row CANVAS_ROWS+3+ → input prompt area
 */
#define PANEL_COL   (CANVAS_COLS + 3)   /* left edge of right panel      */
#define PANEL_W     30                  /* width of panel content        */
#define STATUS_ROW  (CANVAS_ROWS + 2)   /* single status / hint line     */
#define PROMPT_ROW  (CANVAS_ROWS + 3)   /* first row of input prompts    */

void canvas_init(void);
void canvas_clear(void);
void canvas_set(int row, int col, char ch);
char canvas_get(int row, int col);
int  canvas_in_bounds(int row, int col); /* 1 = valid, 0 = out of bounds */
void canvas_display(void);
void canvas_print_raw(void);

#endif /* CANVAS_H */
