#include "graphics.h"
#include "font.h"

static volatile unsigned int *framebuffer;

static unsigned short pitch;
static unsigned short screen_width;
static unsigned short screen_height;
static unsigned short bpp;

static int cursor_x = 0;
static int cursor_y = 0;

static int cursor_visible = 0;
static int drawn_cursor_x = 0;
static int drawn_cursor_y = 0;

static unsigned int cursor_background[FONT_WIDTH * 2];

#define CONSOLE_COLS (SCREEN_WIDTH / FONT_WIDTH)
#define CONSOLE_ROWS (SCREEN_HEIGHT / FONT_HEIGHT)

static unsigned int saved_cells[CONSOLE_ROWS][CONSOLE_COLS][FONT_WIDTH * FONT_HEIGHT];

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

static void hide_cursor()
{
    if (!cursor_visible)
    {
        return;
    }

    for (int py = 0; py < 2; py++)
    {
        for (int px = 0; px < FONT_WIDTH; px++)
        {
            put_pixel(drawn_cursor_x + px, drawn_cursor_y + py, cursor_background[py * FONT_WIDTH + px]);
        }
    }
}

void save_cell(int col, int row)
{
    unsigned int pixels_per_line = pitch / 4;

    for (int y = 0; y < FONT_HEIGHT; y++)
    {
        for (int x = 0; x < FONT_WIDTH; x++)
        {
            saved_cells[row][col][y * FONT_WIDTH + x] = framebuffer[(row * FONT_HEIGHT + y) * pixels_per_line + (col * FONT_WIDTH + x)];
        }
    }
}

void restore_cell(int col, int row)
{
    for (int y = 0; y < FONT_HEIGHT; y++)
    {
        for (int x = 0; x < FONT_WIDTH; x++)
        {
            put_pixel(col * FONT_WIDTH + x, row * FONT_HEIGHT + y, saved_cells[row][col][y * FONT_WIDTH + x]);
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
        {
            cursor_x -= FONT_WIDTH;

            int col = cursor_x / FONT_WIDTH;
            int row = cursor_y / FONT_HEIGHT;

            restore_cell(col, row);
        }
        return;
    }
    hide_cursor();

    int col = cursor_x / FONT_WIDTH;
    int row = cursor_y / FONT_HEIGHT;

    save_cell(col, row);

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
}

void draw_cursor()
{
    fill_rect(cursor_x, cursor_y + FONT_HEIGHT - 2, FONT_WIDTH, 2, console_color);
}

void cursor_blink(void)
{
    if (cursor_visible)
    {
        for (int py = 0; py < 2; py++)
        {
            for (int px = 0; px < FONT_WIDTH; px++)
            {
                put_pixel(
                    drawn_cursor_x + px,
                    drawn_cursor_y + py,
                    cursor_background[py * FONT_WIDTH + px]
                );
            }
        }

        cursor_visible = 0;
        return;
    }

    drawn_cursor_x = cursor_x;
    drawn_cursor_y = cursor_y + FONT_HEIGHT - 2;

    unsigned int pixels_per_line = pitch / 4;

    for (int py = 0; py < 2; py++)
    {
        for (int px = 0; px < FONT_WIDTH; px++)
        {
            cursor_background[py * FONT_WIDTH + px] =
                framebuffer[
                    (drawn_cursor_y + py) * pixels_per_line +
                    (drawn_cursor_x + px)
                ];
        }
    }

    fill_rect(
        drawn_cursor_x,
        drawn_cursor_y,
        FONT_WIDTH,
        2,
        console_color
    );

    cursor_visible = 1;
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

    cursor_x = 0;
    cursor_y = 0;
}