ASM = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy

BOOT = src/boot/boot.asm
BOOT2 = src/boot/boot2.asm

KERNEL_ENTRY = src/kernel/entry.asm
INTERRUPTS = src/kernel/interrupts.asm
KERNEL_C = src/kernel/kernel.c
LINKER = src/kernel/linker.ld

#DRIVERS C
GRAPHICS_C = src/kernel/drivers/graphics.c
FONT_C = src/kernel/drivers/font.c
KEYBOARD_C = src/kernel/drivers/keyboard.c
IO_C = src/kernel/drivers/io.c
IDT_C = src/kernel/drivers/idt.c
TIMER_C = src/kernel/drivers/timer.c
COMMAND_C = src/kernel/drivers/command.c
RTC_C = src/kernel/drivers/rtc.c
CPU_C = src/kernel/drivers/cpu.c
DISK_C = src/kernel/drivers/disk.c

#DRIVERS C

#FILESYSTEM C
FILESYSTEM_C = src/kernel/filesystem/filesystem.c
#FILESYSTEM C

BOOT_BIN = build/boot.bin
BOOT2_BIN = build/boot2.bin

#KERNEL OBJ
KERNEL_ENTRY_OBJ = build/entry.o
KERNEL_C_OBJ = build/kernel.o
INTERRUPTS_OBJ = build/interrupts.o
#KERNEL OBJ

#DRIVERS OBJ
GRAPHICS_OBJ = build/graphics.o
FONT_OBJ = build/font.o
KEYBOARD_OBJ = build/keyboard.o
IO_OBJ = build/io.o
IDT_OBJ = build/idt.o
TIMER_OBJ = build/timer.o
COMMAND_OBJ = build/command.o
RTC_OBJ = build/rtc.o
CPU_OBJ = build/cpu.o
DISK_OBJ = build/disk.o

#DRIVERS OBJ

#FILESYSTEM OBJ
FILESYSTEM_OBJ = build/filesystem.o
#FILESYSTEM OBJ

KERNEL_ELF = build/kernel.elf
KERNEL_BIN = build/kernel.bin
KERNEL_SECTORS = build/kernel_sectors.bin

OS_IMAGE = build/os-image.bin

.PHONY: all run clean

all: $(OS_IMAGE)

build:
	mkdir -p build

$(KERNEL_SECTORS): $(KERNEL_BIN)
	python3 -c "import os; s=(os.path.getsize('$(KERNEL_BIN)')+511)//512; open('$(KERNEL_SECTORS)','wb').write(bytes([s]))"

$(BOOT_BIN): $(BOOT) | build
	$(ASM) $(BOOT) -f bin -o $(BOOT_BIN)

$(BOOT2_BIN): $(BOOT2) $(KERNEL_BIN) | build
	$(eval SIZE := $(shell wc -c < $(KERNEL_BIN)))
	$(eval SECTORS := $(shell echo $$((($(SIZE) + 511) / 512))))
	$(ASM) $(BOOT2) -f bin -dKERNEL_SECTORS=$(SECTORS) -o $(BOOT2_BIN)

$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY) | build
	$(ASM) $(KERNEL_ENTRY) -f elf32 -o $(KERNEL_ENTRY_OBJ)

$(INTERRUPTS_OBJ): $(INTERRUPTS) | build
	$(ASM) $(INTERRUPTS) -f elf32 -o $(INTERRUPTS_OBJ)

$(KERNEL_C_OBJ): $(KERNEL_C) | build
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(KERNEL_C) -o $(KERNEL_C_OBJ)

$(GRAPHICS_OBJ): $(GRAPHICS_C) | build
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(GRAPHICS_C) -o $(GRAPHICS_OBJ)

$(FONT_OBJ): $(FONT_C) | build
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(FONT_C) -o $(FONT_OBJ)

$(KEYBOARD_OBJ): $(KEYBOARD_C) | build
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(KEYBOARD_C) -o $(KEYBOARD_OBJ)

$(IO_OBJ): $(IO_C) | build
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(IO_C) -o $(IO_OBJ)

$(IDT_OBJ): $(IDT_C) | build
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(IDT_C) -o $(IDT_OBJ)

$(TIMER_OBJ): $(TIMER_C) | build
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(TIMER_C) -o $(TIMER_OBJ)

$(COMMAND_OBJ): $(COMMAND_C) | build
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(COMMAND_C) -o $(COMMAND_OBJ)

$(RTC_OBJ): $(RTC_C) | build
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(RTC_C) -o $(RTC_OBJ)

$(CPU_OBJ): $(CPU_C) | build
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(CPU_C) -o $(CPU_OBJ)

$(DISK_OBJ): $(DISK_C) | build
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(DISK_C) -o $(DISK_OBJ)

$(FILESYSTEM_OBJ): $(FILESYSTEM_C) | build
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(FILESYSTEM_C) -o $(FILESYSTEM_OBJ)

$(KERNEL_ELF): $(KERNEL_ENTRY_OBJ) $(KERNEL_C_OBJ) $(GRAPHICS_OBJ) $(FONT_OBJ) $(KEYBOARD_OBJ) $(IO_OBJ) $(INTERRUPTS_OBJ) $(IDT_OBJ) $(TIMER_OBJ) $(FILESYSTEM_OBJ) $(COMMAND_OBJ) $(RTC_OBJ) $(CPU_OBJ) $(DISK_OBJ) $(LINKER)
	$(LD) -m elf_i386 -T $(LINKER) $(KERNEL_ENTRY_OBJ) $(KERNEL_C_OBJ) $(GRAPHICS_OBJ) $(FONT_OBJ) $(KEYBOARD_OBJ) $(IO_OBJ) $(INTERRUPTS_OBJ) $(IDT_OBJ) $(TIMER_OBJ) $(FILESYSTEM_OBJ) $(COMMAND_OBJ) $(DISK_OBJ) $(RTC_OBJ) $(CPU_OBJ) -o $(KERNEL_ELF)

$(KERNEL_BIN): $(KERNEL_ELF) | build
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_BIN)

$(OS_IMAGE): $(BOOT_BIN) $(BOOT2_BIN) $(KERNEL_BIN)
	cat $(BOOT_BIN) $(BOOT2_BIN) $(KERNEL_BIN) > $(OS_IMAGE)
	truncate -s 1M $(OS_IMAGE)

run: $(OS_IMAGE)
	qemu-system-x86_64 -drive format=raw,file=$(OS_IMAGE),if=ide,index=0

clean:
	rm -rf build 