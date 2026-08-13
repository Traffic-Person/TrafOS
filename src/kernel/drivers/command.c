#include "command.h"
#include "screen.h"

void command_run(char *input)
{
    print("\n", 0x0F);
    if (input == "help")
    {
        print("List of commands:\nempty", 0x0F);
    }
}