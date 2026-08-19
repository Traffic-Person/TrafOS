#include "drivers/graphics.h"

#include "drivers/keyboard.h"
#include "drivers/idt.h"
#include "drivers/timer.h"
#include "drivers/disk.h"

#include "filesystem/filesystem.h"

void kernel_main(void)
{
    graphics_init();

    clear_screen(C_BLACK);

    fill_rect(0, 0, 100, 1080, C_RED);
    fill_rect(100, 0, 100, 1080, C_BLUE);
    fill_rect(200, 0, 100, 1080, C_CYAN);

    print("Welcome to TrafOS\n");

    print("ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n1234567890\n!@#$%^&*()_+-=\|[]{};:',.<>/?\n");

    idt_init();
    timer_init();
    __asm__ volatile("sti"); //start accepting interrupts

    filesystem_start();

    filesystem_print_path();

    while (1)
    {
        __asm__ volatile("hlt");
    }
}