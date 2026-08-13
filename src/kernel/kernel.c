#include "drivers/screen.h"
#include "drivers/keyboard.h"
#include "drivers/idt.h"
#include "drivers/timer.h"
#include "drivers/disk.h"

void kernel_main(void)
{
    screen_clear();
    cursor_disable();
    print("Loaded!\n", 0x0F);
    
    idt_init();
    timer_init();
    __asm__ volatile("sti"); //start accepting interrupts

    unsigned char buffer[512];

    disk_read(0, buffer);

    print("TrafOS$ ", 0x0F);

    while (1) // dont let it end 
    {
        __asm__ volatile("hlt");
    }
}