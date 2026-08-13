#include "keyboard.h"
#include "io.h"
#include "screen.h"
#include "command.h"

char input[128];
int input_pos = 0;

int shift = 0;

char keyboard_scancode_to_char(unsigned char scancode)
{
    switch (scancode)
    {
        // Letters
        case 0x1E: return 'a';
        case 0x30: return 'b';
        case 0x2E: return 'c';
        case 0x20: return 'd';
        case 0x12: return 'e';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x17: return 'i';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';
        case 0x32: return 'm';
        case 0x31: return 'n';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x10: return 'q';
        case 0x13: return 'r';
        case 0x1F: return 's';
        case 0x14: return 't';
        case 0x16: return 'u';
        case 0x2F: return 'v';
        case 0x11: return 'w';
        case 0x2D: return 'x';
        case 0x15: return 'y';
        case 0x2C: return 'z';

        // Numbers
        case 0x02: return '1';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';
        case 0x09: return '8';
        case 0x0A: return '9';
        case 0x0B: return '0';

        // Symbols
        case 0x0C: return '-';
        case 0x0D: return '=';
        case 0x1A: return '[';
        case 0x1B: return ']';
        case 0x2B: return '\\';
        case 0x27: return ';';
        case 0x28: return '\'';
        case 0x29: return '`';
        case 0x33: return ',';
        case 0x34: return '.';
        case 0x35: return '/';

        // Space
        case 0x39: return ' ';

        // Enter
        case 0x1C: return '\n';

        // Backspace
        case 0x0E: return '\b';

        default:
            return 0;
    }
}

void keyboard_handler()
{
    unsigned char scancode = inb(0x60);

    // shift pressed
    if (scancode == 0x2A || scancode == 0x36)
    {
        shift = 1;
        outb(0x20, 0x20);
        return;
    }

    // shift released
    if (scancode == 0xAA || scancode == 0xB6)
    {
        shift = 0;
        outb(0x20, 0x20);
        return;
    }

    // key released
    if (scancode & 0x80)
    {
        outb(0x20, 0x20);
        return;
    }

        char key = keyboard_scancode_to_char(scancode);

    if (key != 0)
    {
        if (shift)
        {
            if (key >= 'a' && key <= 'z')
            {
                key = key - 'a' + 'A';
            }

            switch (key)
            {
                case '1': key = '!'; break;
                case '2': key = '@'; break;
                case '3': key = '#'; break;
                case '4': key = '$'; break;
                case '5': key = '%'; break;
                case '6': key = '^'; break;
                case '7': key = '&'; break;
                case '8': key = '*'; break;
                case '9': key = '('; break;
                case '0': key = ')'; break;

                case '-': key = '_'; break;
                case '=': key = '+'; break;

                case '[': key = '{'; break;
                case ']': key = '}'; break;
                case '\\': key = '|'; break;

                case ';': key = ':'; break;
                case '\'': key = '"'; break;
                case '`': key = '~'; break;

                case ',': key = '<'; break;
                case '.': key = '>'; break;
                case '/': key = '?'; break;
            }
        }

        if (key == '\n')
        {
            input[input_pos] = '\0';

            command_run(input);

            input_pos = 0;

            if (!entering_color)
            {
                print("TrafOS$ ", 0x0F);
            }
        }
        else if (key == '\b')
        {
            if (input_pos > 0)
            {
                input_pos--;
                putchar('\b', 0x0F);
            }
        }
        else
        {
            if (input_pos < 127)
            {
                input[input_pos] = key;
                input_pos++;

                putchar(key, 0x0F);
            }
        }
    }

    outb(0x20, 0x20);
}