#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define FS_TYPE_FILE 1
#define FS_TYPE_DIRECTORY 2

typedef struct 
{
    char name [32];

    // file or dir
    unsigned char type;

    // where it info starts
    unsigned int start_lba;

    // size (bytes)
    unsigned int size;

    // flags
    unsigned char used;
    unsigned char writable;
    unsigned char deletable;

    // parent dir
    unsigned int parent;
} filesystem_entry;

#define FS_MAGIC 0x54465331
typedef struct 
{
    unsigned int magic;

    unsigned int filesystem_start;
    unsigned int entry_start;
    unsigned int entry_count;

    unsigned int data_start;
} filesystem_superblock;

void filesystem_init();
void filesystem_list(unsigned int directory);
extern unsigned int current_dir;
int filesystem_change_dir(char *name);
void filesystem_parent_dir();
void filesystem_print_path();

int filesystem_load_superblock();

void filesystem_save_entries();
int filesystem_load_entries();

int filesystem_delete(char *name);

void filesystem_start();

int filesystem_write_file(char *name, char *data);
int filesystem_read_file(char *name);

int filesystem_read_file_data(char *name, unsigned char *buffer, unsigned int buffer_size);
int filesystem_write_file_data(char *name, unsigned char *data, unsigned int size);

int filesystem_create(
    char *name,
    unsigned char type,
    unsigned int parent,
    unsigned char writable,
    unsigned char deletable
);

#endif