#include "disk.h"
#include "io.h"
#include "graphics.h"

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
        print("ATA: BSY timeout\n");
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
            print("ATA: read error\n");
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
        print("ATA: DRQ timeout\n");
        printnum(status);
        print("\n");
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
        print("ATA: error after read\n");
        disk_hang();
    }
}

void disk_write(unsigned int lba, unsigned char *buffer)
{
    unsigned char status;
    int timeout;

    /*
     * Select primary master using LBA28.
     */
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

    /*
     * 400ns delay.
     */
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);

    /*
     * Wait until the drive is no longer busy.
     */
    timeout = ATA_TIMEOUT;

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
        printerr("ATA: write busy timeout\n");
        disk_hang();
    }

    /*
     * Tell ATA we are writing one sector.
     */
    outb(ATA_SECCOUNT, 1);

    /*
     * Set LBA.
     */
    outb(ATA_LBA_LOW,  lba & 0xFF);
    outb(ATA_LBA_MID,  (lba >> 8) & 0xFF);
    outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);

    /*
     * WRITE SECTORS command.
     */
    outb(ATA_COMMAND, 0x30);

    /*
     * Wait for the drive to request data.
     */
    timeout = ATA_TIMEOUT;

    while (timeout > 0)
    {
        status = inb(ATA_STATUS);

        if (status & STATUS_ERR)
        {
            printerr("ATA: write error\n");
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
        printerr("ATA: write DRQ timeout\n");
        disk_hang();
    }

    /*
     * Send 512 bytes = 256 words.
     */
    for (int i = 0; i < 256; i++)
    {
        unsigned short data;

        data = buffer[i * 2];
        data |= ((unsigned short)buffer[i * 2 + 1]) << 8;

        outw(ATA_DATA, data);
    }

    /*
     * Wait for the write to finish.
     */
    timeout = ATA_TIMEOUT;

    while (timeout > 0)
    {
        status = inb(ATA_STATUS);

        if (status & STATUS_ERR)
        {
            printerr("ATA: write error after data\n");
            disk_hang();
        }

        if (!(status & STATUS_BSY))
        {
            break;
        }

        timeout--;
    }

    if (timeout == 0)
    {
        printerr("ATA: write completion timeout\n");
        disk_hang();
    }

    /*
     * Final status check.
     */
    status = inb(ATA_STATUS);

    if (status & STATUS_ERR)
    {
        printerr("ATA: write failed\n");
        disk_hang();
    }
}