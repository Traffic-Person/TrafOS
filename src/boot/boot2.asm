[org 0x7e00]
bits 16

start:
    ; kernel load code here
    ; load kernel at 0x10000
    mov ax, 0x1000
    mov es, ax
    mov bx, 0x0000

    mov dl, [0x0500]

    mov ah, 0x02
    mov al, KERNEL_SECTORS ; Sectors to load (1 sector is 512 bytes)
    mov ch, 0
    mov cl, 3
    mov dh, 0

    int 0x13
    jc disk_error

    jmp protected_mode_start

disk_error:
    jmp $

protected_mode_start:
    cli

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode

gdt_start:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

bits 32
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ;jump to kernel address at 0x10000
    jmp  0x10000

times 512-($-$$) db 0