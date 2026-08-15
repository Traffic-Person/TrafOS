#include "drivers/screen.h"
#include "drivers/keyboard.h"
#include "drivers/idt.h"
#include "drivers/timer.h"
#include "drivers/disk.h"

#include "filesystem/filesystem.h"

void kernel_main(void)
{
    screen_clear();
    cursor_disable();
    print("Welcome to TrafOS\n", 0x0F);
    
    idt_init();
    timer_init();
    __asm__ volatile("sti"); //start accepting interrupts

    filesystem_start();

    filesystem_print_path();

    while (1) // dont let it end 
    {
        __asm__ volatile("hlt");
    }
}