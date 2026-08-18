#include "graphics.h"

static volatile unsigned int *framebuffer;

static unsigned short pitch;
static unsigned short screen_width;
static unsigned short screen_height;
static unsigned short bpp;

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