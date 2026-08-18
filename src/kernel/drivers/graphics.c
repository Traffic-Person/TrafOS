#include "graphics.h"
#include "font.h"

static volatile unsigned int *framebuffer;

static unsigned short pitch;
static unsigned short screen_width;
static unsigned short screen_height;
static unsigned short bpp;

static int cursor_x = 0;
static int cursor_y = 0;

static unsigned int console_color = C_WHITE;

void graphics_init(void)
{
    framebuffer = (volatile unsigned int *)(*(unsigned int *)0x80400);

    pitch = *(unsigned short *)0x80404;
    screen_width = *(unsigned short *)0x80406;
    screen_height = *(unsigned short *)0x80408;
    bpp = *(unsigned short *)0x8040A;
}

void put_pixel(int x, int y, unsigned int color)
{
    if (x < 0 || y < 0)
    {
        return;
    }
    if (x >= screen_width || y >= screen_height)
    {
        return;
    }

    unsigned int pixels_per_line = pitch / 4;

    framebuffer[y * pixels_per_line + x] = color;
}

void fill_rect(int x, int y, int width, int height, unsigned int color)
{
    for (int py = 0; py < height; py++)
    {
        for (int px = 0; px < width; px++)
        {
            put_pixel(x + px, y + py, color);
        }
    }
}

void draw_char(char c, int x, int y, unsigned int color)
{
    for (int row = 0; row < FONT_HEIGHT; row++)
    {
        unsigned char bits = font[(unsigned char)c][row];

        for (int col = 0; col < FONT_WIDTH; col++)
        {
            if (bits & (1 << (7 - col)))
            {
                put_pixel(x + col, y + row, color);
            }
        }
    }
}

void putchar(char c, unsigned int color)
{
    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y += FONT_HEIGHT;
        return;
    }

    if (c == '\b')
    {
        if (cursor_x >= FONT_WIDTH)
            cursor_x -= FONT_WIDTH;

        return;
    }

    draw_char(c, cursor_x, cursor_y, color);

    cursor_x += FONT_WIDTH;

    if (cursor_x + FONT_WIDTH > SCREEN_WIDTH)
    {
        cursor_x = 0;
        cursor_y += FONT_HEIGHT;
    }
}

void print(const char *str)
{
    while (*str)
    {
        if (*str == '\n')
        {
            cursor_x = 0;
            cursor_y += FONT_HEIGHT;
        }
        else
        {
            draw_char(*str, cursor_x, cursor_y, console_color);

            cursor_x += FONT_WIDTH;

            // Move to next line if we reach the edge.
            if (cursor_x + FONT_WIDTH > SCREEN_WIDTH)
            {
                cursor_x = 0;
                cursor_y += FONT_HEIGHT;
            }
        }

        str++;
    }
}

void printnum(int num)
{
    char buffer[12];
    int i = 0;

    if (num == 0)
    {
        print("0");
        return;
    }

    if (num < 0)
    {
        print("-");
        num = -num;
    }

    while (num > 0)
    {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    while (i > 0)
    {
        i--;

        char str[2];

        str[0] = buffer[i];
        str[1] = '\0';

        print(str);
    }
}

void printerr(const char *str)
{
    print("Err: ");
    print(str);
    print("\n");
}

void clear_screen(unsigned int color)
{
    for (int y = 0; y < screen_height; y++)
    {
        for (int x = 0; x < screen_width; x++)
        {
            put_pixel(x, y, color);
        }
    }
}