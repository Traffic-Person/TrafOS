#include "disk.h"
#include "io.h"
#include "screen.h"

#define ATA_DATA        0x1F0
#define ATA_SECCOUNT    0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE       0x1F6
#define ATA_STATUS      0x1F7
#define ATA_COMMAND     0x1F7

#define ATA_READ        0x20

#define STATUS_ERR      0x01
#define STATUS_DRQ      0x08
#define STATUS_BSY      0x80

#define ATA_TIMEOUT     1000000

void disk_hang()
{
    while (1)
    {
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}

int ata_wait_bsy()
{
    int timeout = ATA_TIMEOUT;

    while (timeout > 0)
    {
        unsigned char status = inb(ATA_STATUS);

        if (!(status & STATUS_BSY))
        {
            return 1;
        }

        timeout--;
    }

    return 0;
}

int ata_wait_drq()
{
    int timeout = ATA_TIMEOUT;

    while (timeout > 0)
    {
        unsigned char status = inb(ATA_STATUS);

        if (status & STATUS_ERR)
        {
            return -1;
        }

        if (!(status & STATUS_BSY) && (status & STATUS_DRQ))
        {
            return 1;
        }

        timeout--;
    }

    return 0;
}

void disk_read(unsigned int lba, unsigned char *buffer)
{
    unsigned char status;
    int timeout;

    // Select primary master
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

    // 400ns delay
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);

    // Wait for BSY to clear
    timeout = 1000000;

    while (timeout > 0)
    {
        status = inb(ATA_STATUS);

        if (!(status & STATUS_BSY))
        {
            break;
        }

        timeout--;
    }

    if (timeout == 0)
    {
        print("ATA: BSY timeout\n", 0x04);
        disk_hang();
    }

    // Set sector count
    outb(ATA_SECCOUNT, 1);

    // Set LBA
    outb(ATA_LBA_LOW, lba & 0xFF);
    outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);

    // Send READ command
    outb(ATA_COMMAND, ATA_READ);

    // Wait for BSY to clear and DRQ to appear
    timeout = 1000000;

    while (timeout > 0)
    {
        status = inb(ATA_STATUS);

        if (status & STATUS_ERR)
        {
            print("ATA: read error\n", 0x04);
            disk_hang();
        }

        if (!(status & STATUS_BSY) && (status & STATUS_DRQ))
        {
            break;
        }

        timeout--;
    }

    if (timeout == 0)
    {
        print("ATA: DRQ timeout\n", 0x04);
        printnum(status, 0x04);
        print("\n", 0x04);
        disk_hang();
    }

    // Read 512 bytes
    for (int i = 0; i < 256; i++)
    {
        unsigned short data = inw(ATA_DATA);

        buffer[i * 2] = data & 0xFF;
        buffer[i * 2 + 1] = (data >> 8) & 0xFF;
    }

    // Final status check
    status = inb(ATA_STATUS);

    if (status & STATUS_ERR)
    {
        print("ATA: error after read\n", 0x04);
        disk_hang();
    }
}