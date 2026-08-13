#include "timer.h"
#include "io.h"
#include "screen.h"

volatile unsigned int ticks = 0;

void timer_handler()
{
    ticks++;

    if (ticks % 50 == 0)
    {
        cursor_blink();
    }

    outb(0x20, 0x20);
}

void timer_init()
{
    // tell pit we are setting the freq
    outb(0x43, 0x36);

    // timer freq
    unsigned short divisor = 1193180 / 100;

    // send the divisor
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}