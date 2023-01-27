#include <ncurses.h>
#include <curses.h>
#include <stdio.h>

int main() {
    WINDOW *w;
    char c;

    w = initscr();
	cbreak();
	noecho();
	int maxlines = LINES - 1;
	mvaddstr(maxlines, 0, "This is test for chat");
    c = getch();
    endwin();

    printf("%dx%d window\n", getmaxx(w), getmaxy(w));
}