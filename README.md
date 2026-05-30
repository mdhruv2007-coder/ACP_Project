# 2D Graphics Editor — ACP Assignment (2nd Sem)
### Reva University | Advanced C Programming

A terminal-based 2D graphics editor written in C using the **ncurses** library.  
Shapes are rendered on a character canvas using `*` and `_` characters.

---

## Build & Run

### Prerequisites
- GCC
- ncurses development library (`libncurses-dev` on Ubuntu/Debian)

```bash
sudo apt install libncurses-dev   # if not already installed
```

### Build
```bash
make
```

### Run
```bash
./editor
```

### Clean
```bash
make clean
```

> **Terminal requirement:** The editor needs at least **115 columns × 37 rows**.  
> If the display looks corrupted, resize your terminal and re-run.

---

## Key Bindings

| Key | Action |
|-----|--------|
| `a` | **Add** a new shape |
| `d` | **Delete** a shape by ID |
| `m` | **Modify** an existing shape by ID |
| `c` | **Clear** the canvas (asks for confirmation) |
| `l` | Refresh the shape panel |
| `q` | **Quit** |

---

## Screen Layout

```
┌──────────────────────────────────────────────────────────────────┐   ┌─ Shapes 2/50 ────────┐
│                                                                  │   │  ID  Type  Coords     │
│                    Canvas (80 × 30)                              │   │  #1  Line  (0,0)->(5,5│
│                                                                  │   │  #2  Rect  (2,2) 5x10 │
│                                                                  │   │                       │
└──────────────────────────────────────────────────────────────────┘   └───────────────────────┘
[ Ready                         ] | a=add  d=del  m=mod  c=clear  l=list  q=quit
Enter type:  _
```

- **Left** — 80×30 drawing canvas with border
- **Right** — live shape panel listing every shape's ID, type, and coordinates
- **Status bar** — current action + key hints
- **Prompt area** — input fields appear here during add/modify/delete

---

## Shapes

| Shape | Inputs |
|-------|--------|
| **Line** | Start (row, col) → End (row, col) |
| **Rectangle** | Top-left (row, col), Height ≥ 2, Width ≥ 2 |
| **Circle** | Centre (row, col), Radius ≥ 1 |
| **Triangle** | Three points P1, P2, P3 (row, col each) |

### Characters used
- Lines → character passed at draw time (default `*`)
- Rectangle → `*` for corners and vertical sides, `_` for top and bottom edges
- Circle → `*`
- Triangle → `*` (three lines connected)

---

## Input Validation (Phase 5)

The editor rejects bad input with a clear error message rather than crashing or drawing garbage:

- **Out-of-bounds coordinates** — any (row, col) outside `0–29` × `0–79` is rejected
- **Rectangle too small** — height or width < 2 is rejected
- **Rectangle off-canvas** — if bottom-right corner exceeds canvas, rejected
- **Circle radius < 1** — rejected; radius that clips the edge is allowed (clipped pixels are silently skipped)
- **Non-numeric input** — `scanw` return value checked; non-numeric cancels the operation without modifying any shape
- **Delete / modify on empty canvas** — detected early with a clear message
- **Invalid ID** — if the ID doesn't exist, the user is told and nothing changes
- **Clear on empty canvas** — skips confirmation and reports "already empty"
- **Canvas full** — once 50 shapes exist, `add_shape` returns -1 and the user is notified

---

## Code Structure

```
ACP_PROJECT_2D_GRAPHICS_EDITOR/
├── Makefile
├── README.md
├── include/
│   ├── canvas.h   — CANVAS_ROWS/COLS, layout constants, canvas API
│   ├── shapes.h   — Shape struct, MAX_SHAPES, shape management API
│   └── ui.h       — ui_run, ui_status declarations
└── src/
    ├── main.c     — ncurses init, calls canvas_init → shapes_init → ui_run
    ├── canvas.c   — 2D char array, canvas_set/get/clear/display/in_bounds
    ├── shapes.c   — drawing primitives, shape store, panel renderer
    └── ui.c       — menu system, input prompts, validation, event loop
```

### Module responsibilities

**`canvas.c`**  
Owns the `canvas[CANVAS_ROWS][CANVAS_COLS]` char array.  
`canvas_set` / `canvas_get` are the only write points — all drawing goes through here.  
`canvas_in_bounds` is used by `ui.c` to validate coordinates before accepting them.

**`shapes.c`**  
Stores up to `MAX_SHAPES` Shape objects in a static array.  
`add_shape` finds the first free slot, assigns an auto-incremented ID, renders the shape, and calls `canvas_display`.  
`redraw_all` clears the canvas and re-renders every active shape — used after delete and modify so overlapping shapes stay correct.  
`panel_draw_shapes` renders the live shape list into the right-side panel without touching the canvas.

**`ui.c`**  
Implements four menus (add / delete / modify / clear) plus the main `getch` loop.  
All user input goes through `prompt_int` which checks `scanw`'s return value.  
Coordinate validity is checked with `canvas_in_bounds` before any shape is committed.  
Errors are shown in the prompt area with a "press any key" pause; the shape store is never modified on an error path.

**`main.c`**  
12 lines. Initialises ncurses, calls the three init functions, starts `ui_run`, then calls `endwin` on exit.

---

## Git Workflow

Daily commits are made to keep a verifiable timestamp history:

```bash
git add -A
git commit -m "descriptive message"
git push origin main
```

Repository is public — the teacher can view it without logging in.

---

## Author

**Dhruv M** | 2nd Semester | Reva University  
Course: Advanced C Programming (ACP)
