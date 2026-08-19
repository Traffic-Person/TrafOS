# TrafOS

This is a x86 32 bit operating system written as a learning project of kernels and operating systems in general.

TrafOS currently targets 32-bit x86 systems and is primarily tested using QEMU.

It has a command-line interface similar to Linux, with commands such as `ls`, `cat`, `mkdir`, etc. (more on that later).

## What is an operating system?

An operating system (OS) is system software that manages computer hardware and software resources and provides an interface for users and applications to interact with the computer.

## How this operating system works

First, the BIOS looks for a bootable device and loads its first sector (the boot sector) into memory at address 0x7C00. The boot sector is 512 bytes and must end with the boot signature 0x55 0xAA.

Then the BIOS executes the boot sector, which loads the second-stage bootloader. The second stage sets up 32-bit protected mode because the BIOS starts the CPU in 16-bit real mode.

In my case, stage 2 searches for a VBE video mode with a resolution of 1920×1080. Once it finds a suitable video mode, it loads the kernel at memory address 0x10000 and transfers control to it.

The kernel is basically the main boss of the operating system. Its tasks include reading keyboard input, displaying text on the screen, and interpreting commands entered by the user.

## What this operating system is written in

The kernel is primarily written in C. The bootloader stages and kernel entry point are written in x86 Assembly.

## Functions

- `help`: displays all available commands
- `about`: displays information about the system
- `clear`: clears the screen
- `echo`: prints a string
- `time`: displays the current time
- `cpu`: displays the CPU vendor
- `uptime`: displays the system uptime
- `ls`: lists files and directories
- `cd`: changes the current directory
- `mkdir`: creates a new directory
- `rm`: deletes a file or empty directory
- `touch`: creates a new file
- `write`: writes data to a file
- `cat`: reads and displays file data
- `fill_rect`: draws a filled rectangle
- `reboot`: reboots the system
- `quit`: shuts down the system

TrafOS supports persistent storage for files, directories, and file data. This means that data is preserved when the system is rebooted.

## How to compile

First, clone the repository and navigate into the project directory:

```bash
git clone https://github.com/Traffic-Person/TrafOS.git
cd TrafOS
```

This project requires several system packages to build and run. On Debian or Ubuntu, install the required system packages with:

```bash
sudo apt install build-essential gcc-multilib nasm binutils python3 qemu-system-x86
```

Once the dependencies are installed, build TrafOS by running:

```bash
make
```

## How to run

### Option A

On Debian or Ubuntu, install qemu with:

```bash
sudo apt install qemu-system-x86
```

Then download os-image.iso from [here](https://github.com/Traffic-Person/TrafOS/releases/tag/release) and in your terminal run:

```bash
qemu-system-x86_64 -drive format=raw,file="os-image.iso",if=ide,index=0
```

### Option B

After compiling run:

```bash
make run
```

## Screenshots

<img width="373" height="302" alt="Screenshot 2026-08-19 160003" src="https://github.com/user-attachments/assets/4c6d65af-5c85-4ad3-bb30-2e86879842e7" />
<img width="746" height="555" alt="image" src="https://github.com/user-attachments/assets/a1a8b528-57d7-4cfd-a0e3-3542960edd97" />
<img width="326" height="217" alt="image" src="https://github.com/user-attachments/assets/1b3e90ab-596d-45e3-b1b6-da27789806cd" />

## Credits

Resources:
[OS Dev Wiki](https://wiki.osdev.org/Expanded_Main_Page)
