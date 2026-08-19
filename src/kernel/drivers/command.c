#include "command.h"
#include "graphics.h"
#include "io.h"
#include "rtc.h"
#include "cpu.h"
#include "timer.h"

#include "../filesystem/filesystem.h"


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

    return a[i] == '\0' && b[i] == '\0';
}


int hex_to_number(char *text)
{
    int result = 0;

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

unsigned int parse_hex(const char *str)
{
    unsigned int value = 0;

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        str += 2;

    while (*str)
    {
        char c = *str;

        value <<= 4;

        if (c >= '0' && c <= '9')
            value += c - '0';
        else if (c >= 'a' && c <= 'f')
            value += c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            value += c - 'A' + 10;
        else
            break;

        str++;
    }

    return value;
}

void execute_test_program(char *name)
{
    unsigned char buffer[512];

    if (!filesystem_read_file_data(name, buffer, 512))
    {
        printerr("could not load executable\n");
        return;
    }

    unsigned char *program = (unsigned char *)0x20000;

    for (unsigned int i = 0; i < 512; i++)
    {
        program[i] = buffer[i];
    }

    print("Program loaded at 0x20000\n");
}

int parse_number(const char **str)
{
    int number = 0;

    while (**str >= '0' && **str <= '9')
    {
        number = number * 10 + (**str - '0');
        (*str)++;
    }

    return number;
}

void command_run(char *input)
{
    char command[32];
    char argument[96];

    int i = 0;
    int j = 0;

    print("\n");


    /*
     * Parse command
     *
     * Example:
     *
     * mkdir test
     *
     * command  = "mkdir"
     * argument = "test"
     */

    while (input[i] != ' ' &&
           input[i] != '\0' &&
           j < 31)
    {
        command[j] = input[i];

        i++;
        j++;
    }

    command[j] = '\0';


    /*
     * Skip spaces between command and argument.
     */

    while (input[i] == ' ')
    {
        i++;
    }


    /*
     * Get argument.
     */

    j = 0;

    while (input[i] != '\0' && j < 95)
    {
        argument[j] = input[i];

        i++;
        j++;
    }

    argument[j] = '\0';


    /*
     * COMMANDS
     */


    if (strcmp(command, "help"))
    {
        print("List of commands:\n");

        print("help:      this command\n");                       // 1
        print("about:     about this system\n");
        print("clear:     clears the screen\n");
        print("echo:      prints a string\n");
        print("time:      prints the time\n");
        print("cpu:       prints the CPU vendor\n");
        print("uptime:    prints uptime\n");
        print("ls:        lists files and directories\n");
        print("cd:        changes directory\n");
        print("mkdir:     makes a new directory\n");
        print("rm:        deletes a file or empty directory\n");
        print("touch:     makes a new file\n");
        print("write:     writes data to a file\n");
        print("cat:       reads file data\n");
        print("fill_rect: draws a filled rectangle\n");
        print("reboot:    reboots the system\n");
        print("quit:      shuts down the system\n");               // 17
    }


    else if (strcmp(command, "clear"))
    {
        clear_screen(C_BLACK);
    }

    else if (strcmp(command, "about"))
    {
        print("TrafOS, made by Traffic person x86_32 bit system\n");
        print("github.com/Traffic-Person/TrafOS\n");
    }


    else if (strcmp(command, "quit"))
    {
        print("Shutting down...\n");

        while (1)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }


    else if (strcmp(command, "echo"))
    {
        print(argument);
        print("\n");
    }


    else if (strcmp(command, "reboot"))
    {
        print("Rebooting...\n");

        outb(0x64, 0xFE);

        while (1)
        {
            __asm__ volatile("hlt");
        }
    }


    else if (strcmp(command, "time"))
    {
        int hours = rtc_hours();
        int minutes = rtc_minutes();
        int seconds = rtc_seconds();

        printnum(hours);
        print(":");

        if (minutes < 10)
        {
            print("0");
        }

        printnum(minutes);
        print(":");

        if (seconds < 10)
        {
            print("0");
        }

        printnum(seconds);
        print("\n");
    }


    else if (strcmp(command, "cpu"))
    {
        char vendor[13];

        cpu_vendor(vendor);

        print("CPU: ");
        print(vendor);
        print("\n");
    }


    else if (strcmp(command, "uptime"))
    {
        unsigned int seconds = timer_ticks / 100;

        print("Uptime: ");
        printnum(seconds);
        print(" seconds\n");
    }


    else if (strcmp(command, "ls"))
    {
        filesystem_list(current_dir);
    }


    else if (strcmp(command, "cd"))
    {
        if (argument[0] == '\0')
        {
            printerr("missing directory\n");
        }

        else if (strcmp(argument, ".."))
        {
            filesystem_parent_dir();
        }

        else
        {
            if (filesystem_change_dir(argument))
            {
                print("Changed directory to: ");
                print(argument);
                print("\n");
            }
            else
            {
                printerr("directory '");
                print(argument);
                print("' not found\n");
            }
        }
    }


    else if (strcmp(command, "mkdir"))
    {
        if (argument[0] == '\0')
        {
            printerr("missing directory name\n");
        }

        else if (
            filesystem_create(
                argument,
                FS_TYPE_DIRECTORY,
                current_dir,
                1,
                1
            ) >= 0)
        {
            print("Directory '");
            print(argument);
            print("' created\n");
        }

        else
        {
            printerr("could not create directory\n");
        }
    }


    else if (strcmp(command, "rm"))
    {
        if (argument[0] == '\0')
        {
            printerr("missing file or directory name\n");
        }

        else if (filesystem_delete(argument))
        {
            print("Deleted '");
            print(argument);
            print("'\n");
        }

        else
        {
            printerr("could not delete '");
            print(argument);
            print("'\n");
        }
    }

    else if (strcmp(command, "touch"))
    {
        if (argument[0] == '\0')
        {
            printerr("missing file name\n");
        }
        else if (filesystem_create(argument, FS_TYPE_FILE, current_dir, 1, 1) >= 0)
        {
            print("File '");
            print(argument);
            print("' created\n");
        }
        else
        {
            printerr("could not create file\n");
        }
    }

    else if (strcmp(command, "write"))
    {
        int k = 0;

        while (argument[k] != ' ' &&
            argument[k] != '\0')
        {
            k++;
        }

        if (argument[k] == '\0')
        {
            printerr("usage: write <file> <data>\n");
        }
        else
        {
            argument[k] = '\0';

            char *filename = argument;
            char *data = argument + k + 1;

            if (data[0] == '\0')
            {
                printerr("missing data\n");
            }
            else if (filesystem_write_file(filename, data))
            {
                print("File written\n");
            }
            else
            {
                printerr("could not write file\n");
            }
        }
    }

    else if (strcmp(command, "cat"))
    {
        if (argument[0] == '\0')
        {
            printerr("missing file name\n");
        }
        else if (!filesystem_read_file(argument))
        {
            printerr("could not read file\n");
        }
    }

    else if (strcmp(command, "loadtest"))
    {
        unsigned char buffer[512];

        if (filesystem_read_file_data(argument, buffer, 512))
        {
            print("Loaded file into memory\n");

            print("Contents: ");

            print("Bytes: ");

            for (unsigned int i = 0; i < 16; i++)
            {
                printnum(buffer[i]);
                print(" ");
            }

            print("\n");
        }
        else
        {
            printerr("could not load file\n");
        }
    }

    else if (strcmp(command, "exec"))
    {
        if (argument[0] == '\0')
        {
            printerr("missing executable name\n");
        }
        else
        {
            execute_test_program(argument);
        }
    }

    else if (strcmp(command, "fill_rect"))
    {
        if (argument[0] == '\0')
        {
            printerr("usage: fill_rect <x>, <y>, <width>, <height>, <color>\n");
            print("Colors: 0x11223344, 1 is alpha, 2 is red, 3 is green, 4 is blue\n");
        }
        const char *p = argument;

        int x = parse_number(&p);

        while (*p == ' ' || *p == ',')
        {
            p++;
        }

        int y = parse_number(&p);

        while (*p == ' ' || *p == ',')
        {
            p++;
        }

        int width = parse_number(&p);

        while (*p == ' ' || *p == ',')
        {
            p++;
        }

        int height = parse_number(&p);

        while (*p == ' ' || *p == ',')
        {
            p++;
        }

        unsigned int color = parse_hex(p);

        fill_rect(x, y, width, height, color);
    }

    else
    {
        printerr("unknown command '");
        print(command);
        print("'\n");
    }
}