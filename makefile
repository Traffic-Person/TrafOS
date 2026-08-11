ASM = nasm

BOOT = src/boot/boot.asm
BOOT2 = src/boot/boot2.asm

BOOT_BIN = build/boot.bin
BOOT2_BIN = build/boot2.bin
OS_IMAGE = build/os-image.bin

all: $(OS_IMAGE)

build:
	mkdir -p build

$(BOOT_BIN): $(BOOT) | build
	$(ASM) $(BOOT) -f bin -o $(BOOT_BIN)

$(BOOT2_BIN): $(BOOT2) | build
	$(ASM) $(BOOT2) -f bin -o $(BOOT2_BIN)

$(OS_IMAGE): $(BOOT_BIN) $(BOOT2_BIN)
	cat $(BOOT_BIN) $(BOOT2_BIN) > $(OS_IMAGE)

run: $(OS_IMAGE)
	qemu-system-x86_64 -drive format=raw,file=$(OS_IMAGE)

clean:
	rm -rf build