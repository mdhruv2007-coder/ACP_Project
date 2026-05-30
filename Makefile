CC     = gcc
CFLAGS = -Wall -Wextra -g
LIBS   = -lncurses
TARGET = editor
SRCS   = src/main.c src/canvas.c src/shapes.c src/ui.c

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)
