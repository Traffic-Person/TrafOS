#include "filesystem.h"

#include "../drivers/screen.h"
#include "../drivers/disk.h"

#define FS_MAX_ENTRIES 64

#define FS_START_LBA 64
#define FS_SECTOR_SIZE 512
#define FS_ENTRY_SECTORS 6

filesystem_superblock superblock;
filesystem_entry entries[FS_MAX_ENTRIES];

unsigned int current_dir = 0;

int fsstrcmp(char *a, char *b)
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

void filesystem_save_superblock()
{
    unsigned char buffer[512];

    for (int i = 0; i < 512; i++)
    {
        buffer[i]= 0;
    }

    unsigned int *data = (unsigned int *)buffer;

    data[0] = superblock.magic;
    data[1] = superblock.filesystem_start;
    data[2] = superblock.entry_start;
    data[3] = superblock.entry_count;
    data[4] = superblock.data_start;

    disk_write(FS_START_LBA, buffer);
}

int filesystem_load_superblock()
{
    unsigned char buffer[512];

    disk_read(FS_START_LBA, buffer);

    unsigned int *data = (unsigned int *)buffer;

    if (data[0] != FS_MAGIC)
    {
        return 0;
    }

    superblock.magic = data[0];
    superblock.filesystem_start = data[1];
    superblock.entry_start = data[2];
    superblock.entry_count = data[3];
    superblock.data_start = data[4];

    return 1;
}

void filesystem_save_entries()
{
    unsigned char *data = (unsigned char *)entries;

    for (int sector = 0; sector < FS_ENTRY_SECTORS; sector++)
    {
        disk_write(superblock.entry_start + sector, data + (sector * FS_SECTOR_SIZE));
    }
}

int filesystem_load_entries()
{
    unsigned char *data = (unsigned char *)entries;

    for (int sector = 0; sector < FS_ENTRY_SECTORS; sector++)
    {
        disk_read(superblock.entry_start + sector, data + (sector * FS_SECTOR_SIZE));
    }

    return 1;
}

void filesystem_init()
{
    superblock.magic = FS_MAGIC;
    superblock.filesystem_start = FS_START_LBA;
    superblock.entry_start = FS_START_LBA + 1;
    superblock.entry_count = FS_MAX_ENTRIES;
    superblock.data_start = FS_START_LBA + 1 + FS_ENTRY_SECTORS;

    for (int i = 0; i < FS_MAX_ENTRIES; i++)
    {
        entries[i].used = 0;
    }

    filesystem_create("/", FS_TYPE_DIRECTORY, 0, 1, 0);
    filesystem_create("data", FS_TYPE_DIRECTORY, 0, 1, 1);
    filesystem_create("hello.txt", FS_TYPE_FILE, 1, 1, 1);

    filesystem_save_superblock();
    filesystem_save_entries();
}

void filesystem_list(unsigned int directory)
{
    for (int i = 0; i < FS_MAX_ENTRIES; i++)
    {
        if (!entries[i].used)
        {
            continue;
        }
        if (entries[i].parent != directory || i == directory)
        {
            continue;
        }

        if (entries[i].type == FS_TYPE_DIRECTORY)
        {
            print(entries[i].name, 0x0B);
            print("/", 0x0B);
        }
        else 
        {
            print(entries[i].name, 0x0F);
        }

        print("\n", 0x0F);
    }
}

int filesystem_change_dir(char *name)
{
    for (int i = 0; i < FS_MAX_ENTRIES; i++)
    {
        if (!entries[i].used)
        {
            continue;
        }

        if (entries[i].parent != current_dir)
        {
            continue;
        }

        if (entries[i].type != FS_TYPE_DIRECTORY)
        {
            continue;
        }

        if (!fsstrcmp(entries[i].name, name))
        {
            continue;
        }

        current_dir = i;
        return 1;
    }

    return 0;
}

void filesystem_parent_dir()
{
    current_dir = entries[current_dir].parent;
}

void filesystem_print_path()
{
    unsigned int path[FS_MAX_ENTRIES];
    int path_len = 0;

    unsigned int current = current_dir;

    while (1)
    {
        path[path_len] = current;
        path_len++;

        if (current == 0)
        {
            break;
        }
        
        current = entries[current].parent;
    }

    print("TrafOS ", 0x0F);

    print("/", 0x0F);

    for (int i = path_len - 2; i >= 0; i--)
    {
        print(entries[path[i]].name, 0x0F);

        if (i > 0)
        {
            print("/", 0x0F);
        }
    }

    print(" $ ", 0x0F);
}

int filesystem_create(
    char *name,
    unsigned char type,
    unsigned int parent,
    unsigned char writable,
    unsigned char deletable
)
{
    for (int i = 0; i < FS_MAX_ENTRIES; i++)
    {
        if (entries[i].used)
        {
            continue;
        }

        entries[i].used = 1;

        int j = 0;

        while (name[j] != '\0' && j < 31)
        {
            entries[i].name[j] = name[j];
            j++;
        }

        entries[i].name[j] = '\0';

        entries[i].type = type;
        entries[i].start_lba = 0;
        entries[i].size = 0;
        entries[i].writable = writable;
        entries[i].deletable = deletable;
        entries[i].parent = parent;

        filesystem_save_entries();

        return i;
    }

    return -1;
}

int filesystem_delete(char *name)
{
    for (int i = 0; i < FS_MAX_ENTRIES; i++)
    {
        if (!entries[i].used)
        {
            continue;
        }
        if (entries[i].parent != current_dir)
        {
            continue;
        }
        if (!fsstrcmp(entries[i].name, name))
        {
            continue;
        }
        // dont delete root
        if (i == 0)
        {
            return 0;
        }
        if (entries[i].type == FS_TYPE_DIRECTORY)
        {
            for (int j = 0; j < FS_MAX_ENTRIES; j++)
            {
                if (!entries[j].used)
                {
                    continue;
                }
                if (entries[j].parent == (unsigned int)i)
                {
                    return 0;
                }
            }
        }
        entries[i].used = 0;

        filesystem_save_entries();

        return 1;
    }

    return 0;
}

int filesystem_string_length(char *text)
{
    int len = 0;

    while (text[len] != '\0')
    {
        len++;
    }

    return len;
}

int filesystem_find_free_data_sector()
{
    unsigned int lba = superblock.data_start;

    for (int i = 0; i < FS_MAX_ENTRIES; i++)
    {
        if (!entries[i].used)
        {
            continue;
        }
        if (entries[i].type != FS_TYPE_FILE)
        {
            continue;
        }
        if (entries[i].start_lba == lba)
        {
            lba++;
            i = -1;
        }
    }

    return lba;
}

int filesystem_read_file(char *name)
{
    for (int i = 0; i < FS_MAX_ENTRIES; i++)
    {
        if (!entries[i].used)
        {
            continue;
        }

        if (entries[i].parent != current_dir)
        {
            continue;
        }

        if (!fsstrcmp(entries[i].name, name))
        {
            continue;
        }

        if (entries[i].type != FS_TYPE_FILE)
        {
            return 0;
        }

        if (entries[i].size == 0)
        {
            return 1;
        }

        unsigned char buffer[FS_SECTOR_SIZE];

        disk_read(entries[i].start_lba, buffer);

        for (unsigned int j = 0; j < entries[i].size; j++)
        {
            putchar(buffer[j], 0x0F);
        }

        print("\n", 0x0F);

        return 1;
    }

    return 0;
}

int filesystem_write_file(char *name, char *data)
{
    int length = filesystem_string_length(data);

    if (length > 511)
    {
        return 0;
    }

    for (int i = 0; i < FS_MAX_ENTRIES; i++)
    {
        if (!entries[i].used)
        {
            continue;
        }

        if (entries[i].parent != current_dir)
        {
            continue;
        }

        if (!fsstrcmp(entries[i].name, name))
        {
            continue;
        }

        if (entries[i].type != FS_TYPE_FILE)
        {
            return 0;
        }

        if (!entries[i].writable)
        {
            return 0;
        }

        /*
         * Allocate a data sector if this file
         * doesn't have one yet.
         */
        if (entries[i].start_lba == 0)
        {
            entries[i].start_lba =
                filesystem_find_free_data_sector();
        }

        unsigned char buffer[FS_SECTOR_SIZE];

        /*
         * Clear the whole sector.
         */
        for (int j = 0; j < FS_SECTOR_SIZE; j++)
        {
            buffer[j] = 0;
        }

        /*
         * Copy the data into the sector.
         */
        for (int j = 0; j < length; j++)
        {
            buffer[j] = data[j];
        }

        for (int j = 0; j < length; j++)
        {
            putchar(buffer[j], 0x0E);
        }

        print("\n", 0x0E);

        /*
         * Write sector.
         */
        disk_write(entries[i].start_lba, buffer);

        /*
         * Update file size.
         */
        entries[i].size = length;

        /*
         * Save metadata.
         */
        filesystem_save_entries();

        return 1;
    }

    return 0;
}

void filesystem_start()
{
    if (filesystem_load_superblock())
    {
        print("Filesystem found!\n", 0x0A);

        if (filesystem_load_entries())
        {
            print("Filesystem entries loaded!\n", 0x0A);
        }
        else
        {
            print("Failed to load filesystem entries!\n", 0x04);
        }
    }
    else
    {
        print("No filesystem found!\n", 0x04);

        filesystem_init();

        print("Filesystem created!\n", 0x0A);
    }

}