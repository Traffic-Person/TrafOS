#include "rtc.h"
#include "io.h"

unsigned char rtc_read(unsigned char reg)
{
    outb(0x70, reg);
    return inb(0x71);
}

unsigned char bcd_to_binary(unsigned char value)
{
    return (value & 0x0F) + ((value >> 4) * 10);
}

int rtc_hours()
{
    return bcd_to_binary(rtc_read(0x04));
}

int rtc_minutes()
{
    return bcd_to_binary(rtc_read(0x02));
}

int rtc_seconds()
{
    return bcd_to_binary(rtc_read(0x00));
}