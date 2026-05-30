#ifndef SHAPES_H
#define SHAPES_H

#define MAX_SHAPES 50

/* Shape type identifiers */
#define SHAPE_LINE      1
#define SHAPE_RECTANGLE 2
#define SHAPE_CIRCLE    3
#define SHAPE_TRIANGLE  4

/* ── Shape struct ────────────────────────────────────────────────────── */
typedef struct {
    int  id;        /* unique ID assigned on add          */
    int  type;      /* SHAPE_LINE / RECT / CIRCLE / TRI   */
    int  r1, c1;    /* point 1  (all shapes use this)     */
    int  r2, c2;    /* point 2  (line, rect, tri)         */
    int  r3, c3;    /* point 3  (triangle only)           */
    int  radius;    /* circle only                        */
    char ch;        /* character used to draw             */
    int  active;    /* 1 = exists, 0 = deleted            */
} Shape;

/* ── Drawing primitives ──────────────────────────────────────────────── */
void draw_line(int r1, int c1, int r2, int c2, char ch);
void draw_rectangle(int r, int c, int h, int w);
void draw_circle(int cr, int cc, int radius);
void draw_triangle(int r1, int c1, int r2, int c2, int r3, int c3);

/* ── Object management ───────────────────────────────────────────────── */
void   shapes_init(void);
int    add_shape(Shape s);          /* returns assigned id, -1 if full  */
int    delete_shape(int id);        /* returns 0 ok, -1 not found       */
Shape *find_shape(int id);          /* returns pointer or NULL          */
int    modify_shape(int id, Shape updated); /* 0 ok, -1 not found      */
void   redraw_all(void);            /* clear canvas, redraw every shape */
void   list_shapes(void);           /* kept for compatibility           */
void   panel_draw_shapes(void);     /* render shape list in right panel */
int    get_shape_count(void);

#endif /* SHAPES_H */
