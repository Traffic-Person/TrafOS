#include "command.h"
#include "screen.h"
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

void command_run(char *input)
{
    char command[32];
    char argument[96];

    int i = 0;
    int j = 0;

    print("\n", 0x0F);


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

        print("help:   this command\n");                       // 1
        print("about:  about this system\n");
        print("clear:  clears the screen\n");
        print("color:  changes text color\n");
        print("echo:   prints a string\n");
        print("time:   prints the time\n");
        print("cpu:    prints the CPU vendor\n");
        print("uptime: prints uptime\n");
        print("ls:     lists files and directories\n");
        print("cd:     changes directory\n");
        print("mkdir:  makes a new directory\n");
        print("rm:     deletes a file or empty directory\n");
        print("touch:  makes a new file\n");
        print("write:  writes data to a file\n");
        print("cat:    reads file data\n");
        print("reboot: reboots the system\n");
        print("quit:   shuts down the system\n");               // 17
    }


    else if (strcmp(command, "clear"))
    {
        screen_clear();
    }


    else if (strcmp(command, "color"))
    {
        if (argument[0] == '\0')
        {
            printerr("missing color\n");
        }
        else
        {
            int color = hex_to_number(argument);

            if (color >= 0 && color <= 0x0F)
            {
                screen_set_color(color);

                print("Color changed!\n", color);
            }
            else if (strcmp(argument, "default"))
            {
                screen_set_color(0x0F);

                print("Color changed!\n", 0x0F);
            }
            else
            {
                printerr("invalid VGA color!\n");
            }
        }
    }


    else if (strcmp(command, "about"))
    {
        print("TrafOS, made by Traffic person x86_32 bit system\n", 0x0F);
        print("github.com/Traffic-Person/TrafOS\n", 0x0F);
    }


    else if (strcmp(command, "quit"))
    {
        print("Shutting down...\n", 0x0F);

        while (1)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }


    else if (strcmp(command, "echo"))
    {
        print(argument, 0x0F);
        print("\n", 0x0F);
    }


    else if (strcmp(command, "reboot"))
    {
        print("Rebooting...\n", 0x0F);

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


    else if (strcmp(command, "cpu"))
    {
        char vendor[13];

        cpu_vendor(vendor);

        print("CPU: ", 0x0F);
        print(vendor, 0x0F);
        print("\n", 0x0F);
    }


    else if (strcmp(command, "uptime"))
    {
        unsigned int seconds = timer_ticks / 100;

        print("Uptime: ", 0x0F);
        printnum(seconds, 0x0F);
        print(" seconds\n", 0x0F);
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
                print("Changed directory to: ", 0x0F);
                print(argument, 0x0F);
                print("\n", 0x0F);
            }
            else
            {
                printerr("directory '");
                print(argument, 0x04);
                print("' not found\n", 0x04);
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
            print("Directory '", 0x0F);
            print(argument, 0x0F);
            print("' created\n", 0x0F);
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
            print("Deleted '", 0x0F);
            print(argument, 0x0F);
            print("'\n", 0x0F);
        }

        else
        {
            printerr("could not delete '");
            print(argument, 0x04);
            print("'\n", 0x04);
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
            print("File '", 0x0F);
            print(argument, 0x0F);
            print("' created\n", 0x0F);
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
                print("File written\n", 0x0F);
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
            print("Loaded file into memory\n", 0x0A);

            print("Contents: ", 0x0F);

            print("Bytes: ", 0x0F);

            for (unsigned int i = 0; i < 16; i++)
            {
                printnum(buffer[i], 0x0F);
                print(" ", 0x0F);
            }

            print("\n", 0x0F);
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

    else
    {
        printerr("unknown command '");
        print(command, 0x04);
        print("'\n", 0x04);
    }
}