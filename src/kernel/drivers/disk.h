#ifndef DISK_H
#define DISK_H

void disk_read(unsigned int lba, unsigned char *buffer);
void disk_write(unsigned int lba, unsigned char *buffer);

#endif