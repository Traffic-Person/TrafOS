[org 0x8000]

mov si, disk_load_s

print:
    mov ah, 0x0e
    mov al, [si]

    cmp al, 0
    je keyboard

    int 0x10
    inc si
    jmp print

keyboard:
    mov ah, 0
    int 0x16

    mov ah, 0x0e
    int 0x10

    jmp keyboard

disk_load_s:
    db "sector 2 loaded successfully", 0