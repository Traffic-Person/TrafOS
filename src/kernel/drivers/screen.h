#ifndef SCREEN_H
#define SCREEN_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void print(char *text, char color);
void putchar(char c, char color);
void screen_clear();
void cursor_blink();
void cursor_draw();
void cursor_disable();

#endif