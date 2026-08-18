#ifndef GRAPHICS_H
#define GRAPHICS_H

#define C_BLACK       0x00000000
#define C_WHITE       0x00FFFFFF

#define C_RED         0x00FF0000
#define C_GREEN       0x0000FF00
#define C_BLUE        0x000000FF

#define C_YELLOW      0x00FFFF00
#define C_CYAN        0x0000FFFF
#define C_MAGENTA     0x00FF00FF

#define C_GRAY        0x007F7F7F
#define C_DARK_GRAY   0x003F3F3F

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

void graphics_init(void);

void put_pixel(int x, int y, unsigned int color);

void fill_rect(int x, int y, int width, int height, unsigned int color);

void draw_char(char c, int x, int y, unsigned int color);

void print(const char *str);

void printnum(int num);

void printerr(const char *str);

void putchar(char c, unsigned int color);

void clear_screen(unsigned int color);

#endif