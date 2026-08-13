#include "idt.h"
#include "io.h"

extern void timer_interrupt();
extern void keyboard_interrupt();

struct IDTEntry
{
    unsigned short offset_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short offset_high;
} __attribute__((packed));

struct IDTPointer
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct IDTEntry idt[256];
struct IDTPointer idt_pointer;

void idt_set_gate(int number, unsigned int handler)
{
    idt[number].offset_low = handler & 0xFFFF;
    idt[number].selector = 0x08;
    idt[number].zero = 0;
    idt[number].type_attr = 0x8E;
    idt[number].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_load()
{
    __asm__ volatile("lidt %0" : : "m"(idt_pointer));
}

void pic_remap()
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0x00);
    outb(0xA1, 0x40);
}

void idt_init()
{
    pic_remap();

    idt_pointer.limit = sizeof(idt) - 1;
    idt_pointer.base = (unsigned int)&idt;

    for (int i = 0; i < 256; i++)
    {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].type_attr = 0;
        idt[i].offset_high = 0;
    }
    
    idt_set_gate(32, (unsigned int)timer_interrupt);
    idt_set_gate(33, (unsigned int)keyboard_interrupt);

    idt_load();
}