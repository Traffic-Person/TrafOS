#include "command.h"
#include "screen.h"
#include "io.h"
#include "rtc.h"
#include "cpu.h"
#include "timer.h"

int entering_color = 0;

int strcmp(char *a, char *b)
{
    int i = 0;

    while (a[i] != '\0' && b[i] != '\0')
    {
        if (a[i] != b[i])
        {
            return 0;
        }

        i++;
    }

    if (a[i] == '\0' && b[i] == '\0')
    {
        return 1;
    }

    return 0;
}

int hex_to_number(char *text)
{
    int result = 0;

    // Skip 0x
    if (text[0] == '0' && text[1] == 'x')
    {
        text += 2;
    }

    while (*text != '\0')
    {
        result *= 16;

        if (*text >= '0' && *text <= '9')
        {
            result += *text - '0';
        }
        else if (*text >= 'A' && *text <= 'F')
        {
            result += *text - 'A' + 10;
        }
        else if (*text >= 'a' && *text <= 'f')
        {
            result += *text - 'a' + 10;
        }
        else
        {
            return -1;
        }

        text++;
    }

    return result;
}

void command_run(char *input)
{
    print("\n", 0x0F);

    if (entering_color)
    {
        int color = hex_to_number(input);

        if (color >= 0 && color <= 0x0F)
        {
            screen_set_color(color);
            print("Color changed!\n", color);
        }
        else if (strcmp(input, "default"))
        {
            screen_set_color(0x0F);
            print("Color changed!\n", 0x0F);
        }
        else
        {
            print("Err: Invalid VGA color!\n", 0x04);
        }

        entering_color = 0;
    }
    else if (strcmp(input, "help"))
    {
        print("List of commands:\n", 0x0F);
        print("help:   this command\n", 0x0F);
        print("about:  about this system\n", 0x0F);
        print("clear:  clears the screen\n", 0x0F);
        print("color:  changes the color of text with VGA hex codes (enter default for white)\n", 0x0F);
        print("echo:   prints a string to the screen\n", 0x0F);
        print("time:   prints the time to the screen (UTC+0)\n", 0x0F);
        print("cpu:    prints the cpu vendor\n", 0x0F);
        print("uptime: prints uptime\n", 0x0F);
        print("reboot: reboots the system\n", 0x0F);
        print("quit:   shuts down the system\n", 0x0F);
    }
    else if (strcmp(input, "clear"))
    {
        screen_clear();
    }
    else if (strcmp(input, "color"))
    {
        print("Enter VGA hex color: ", 0x0F);
        entering_color = 1;
    }
    else if (strcmp(input, "about"))
    {
        print("TrafOS, made by Traffic person x86_32 bit system\n", 0x0F);
        print("github.com/Traffic-Person/TrafOS\n", 0x0F);
    }
    else if (strcmp(input, "quit"))
    {
        print("Shutting down...\n", 0x0F);
        while (1)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }
    else if (input[0] == 'e' &&
        input[1] == 'c' &&
        input[2] == 'h' &&
        input[3] == 'o' &&
        input[4] == ' ')
        {
            print(input + 5, 0x0F);
            print("\n", 0x0F);
        }
    else if (strcmp(input, "reboot"))
    {
        print("Rebooting...\n", 0x0F);

        outb(0x64, 0xFE);

        while (1)
        {
            __asm__ volatile("hlt");
        }
    }
    else if (strcmp(input, "time"))
    {
        int hours = rtc_hours();
        int minutes = rtc_minutes();
        int seconds = rtc_seconds();

        printnum(hours, 0x0F);
        print(":", 0x0F);

        if (minutes < 10)
        {
            print("0", 0x0F);
        }

        printnum(minutes, 0x0F);
        print(":", 0x0F);

        if (seconds < 10)
        {
            print("0", 0x0F);
        }

        printnum(seconds, 0x0F);
        print("\n", 0x0F);
    }
    else if (strcmp(input, "cpu"))
    {
        char vendor[13];

        cpu_vendor(vendor);

        print("CPU: ", 0x0F);
        print(vendor, 0x0F);
        print("\n", 0x0F);
    }
    else if (strcmp(input, "uptime"))
    {
        unsigned int seconds = timer_ticks / 100;

        print("Uptime: ", 0x0F);
        printnum(seconds, 0x0F);
        print(" seconds\n", 0x0F);
    }
    else
    {
        printerr("Err: ");
        print("unknown command '", 0x0F);
        print(input, 0x0F);
        print("'!\n", 0x0F);
    }
}