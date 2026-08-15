#include "screen.h"
#include "io.h"

volatile unsigned char *vga = (volatile unsigned char *)0xB8000;

int cursor_x = 0;
int cursor_y = 0;

char cursor_visible = 1;
char cursor_under = ' ';
char cursor_color = 0x07;

char current_color = 0x0F;

void screen_set_color(char color)
{
    current_color = color;
}

void cursor_disable()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void cursor_draw()
{
    int i = (cursor_y * VGA_WIDTH + cursor_x) * 2;

    if (cursor_visible)
    {
        vga[i] = '_';
        vga[i+1] = 0x0F;
    }
    else
    {
        vga[i] = cursor_under;
        vga[i+1] = cursor_color;
    }
}

void cursor_blink()
{
    cursor_visible = !cursor_visible;
    cursor_draw();
}

void screen_clear()
{
    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            int i = (y * VGA_WIDTH + x) * 2;

            vga[i] = ' ';
            vga[i+1] = 0x00;
        }
    }

    cursor_x = 0;
    cursor_y = 0;

    cursor_draw();
}

void scroll()
{
    for (int y = 1; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            int from = (y * VGA_WIDTH + x) * 2;
            int to = ((y - 1) * VGA_WIDTH + x) * 2;

            vga[to] = vga[from];
            vga[to + 1] = vga[from + 1];
        }
    }

    for (int x = 0; x < VGA_WIDTH; x++)
    {
        int i = ((VGA_HEIGHT - 1) * VGA_WIDTH + x) * 2;

        vga[i] = ' ';
        vga[i+1] = 0x00;
    }

    cursor_y = VGA_HEIGHT - 1;
}

void putchar(char c, char color)
{
    int old_i = (cursor_y * VGA_WIDTH + cursor_x) * 2;
    vga[old_i] = ' ';
    vga[old_i+1] = color;

    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y++;

        if (cursor_y >= VGA_HEIGHT)
        {
            scroll();
        }
        return;
    }

    if (c == '\b')
    {
        if (cursor_x > 0)
        {
            cursor_x--;

            int i = (cursor_y * VGA_WIDTH + cursor_x) * 2;
            
            vga[i] = ' ';
            vga[i+1] = color;
        }

        else if (cursor_y > 0)
        {
            cursor_y--;
            cursor_x = VGA_WIDTH - 1;

            int i = (cursor_y * VGA_WIDTH + cursor_x) * 2;

            vga[i] = ' ';
            vga[i+1] = color;
        }
        return;
    }

    int i = (cursor_y * VGA_WIDTH + cursor_x) * 2;

    vga[i] = c;
    vga[i+1] = color;

    cursor_x++;

    if (cursor_x >= VGA_WIDTH)
    {
        cursor_x = 0;
        cursor_y++;

        if (cursor_y >= VGA_HEIGHT)
        {
            scroll();
        }
    }

    cursor_draw();
}

void print(char *text, char color) 
{
    while (*text != '\0')
    {
        putchar(*text, current_color);
        text++;
    }
}

void printerr(char *text)
{
    putchar('E', 0x04);
    putchar('r', 0x04);
    putchar('r', 0x04);
    putchar(':', 0x04);
    putchar(' ', 0x04);
    
    while (*text != '\0')
    {
        putchar(*text, 0x0F);
        text++;
    }
}

void printnum(int number, char color)
{
    char buffer[12];
    int i = 0;

    if (number == 0)
    {
        putchar('0', color);
        return;
    }

    while (number > 0)
    {
        buffer[i] = '0' + (number % 10);
        number /= 10;
        i++;
    }

    while (i > 0)
    {
        i--;
        putchar(buffer[i], color);
    }
}