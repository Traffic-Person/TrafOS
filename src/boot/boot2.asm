[org 0x7e00]
bits 16

; ------------------------------------
; Memory locations
; ------------------------------------

VBE_INFO      equ 0x80000
VBE_MODE_INFO equ 0x80200

; Kernel-readable framebuffer info
;
; ES = 0x8000
;
; ES:0400 = physical 0x80400
;
; 0x80400 = framebuffer address
; 0x80404 = pitch
; 0x80406 = width
; 0x80408 = height
; 0x8040A = bpp


start:

    ; --------------------------------
    ; Load kernel at physical 0x10000
    ; --------------------------------

    mov ax, 0x1000
    mov es, ax
    mov bx, 0x0000

    mov dl, [0x0500]

    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0
    mov cl, 4
    mov dh, 0

    int 0x13
    jc disk_error


    ; --------------------------------
    ; Get VBE controller information
    ; --------------------------------

    mov ax, 0x8000
    mov es, ax
    xor di, di

    ; Tell VBE we want VBE 2 information
    mov byte [es:di+0], 'V'
    mov byte [es:di+1], 'B'
    mov byte [es:di+2], 'E'
    mov byte [es:di+3], '2'

    mov ax, 0x4F00
    int 0x10

    cmp ax, 0x004F
    jne vbe_error


    ; --------------------------------
    ; Check returned "VESA" signature
    ; --------------------------------

    mov ax, 0x8000
    mov es, ax
    xor di, di

    cmp byte [es:di+0], 'V'
    jne vbe_error

    cmp byte [es:di+1], 'E'
    jne vbe_error

    cmp byte [es:di+2], 'S'
    jne vbe_error

    cmp byte [es:di+3], 'A'
    jne vbe_error


    ; --------------------------------
    ; Get pointer to VBE mode list
    ;
    ; offset 0x0E = mode-list offset
    ; offset 0x10 = mode-list segment
    ; --------------------------------

    mov bx, [es:0x0E]
    mov dx, [es:0x10]

    mov es, dx
    mov di, bx


    ; --------------------------------
    ; Walk through all VBE modes
    ; --------------------------------

.next_mode:

    mov cx, [es:di]

    cmp cx, 0xFFFF
    je vbe_no_mode

    ; Save mode-list position
    push es
    push di

    ; Save mode number
    push cx

    ; --------------------------------
    ; VBE mode information buffer
    ;
    ; Physical address = 0x80200
    ; ES = 0x8000
    ; DI = 0x0200
    ; --------------------------------

    mov ax, 0x8000
    mov es, ax
    mov di, 0x0200

    ; Ask BIOS for information
    ; about this mode
    mov ax, 0x4F01
    int 0x10

    cmp ax, 0x004F
    jne .mode_failed

    ; BIOS may have changed ES.
    ; Restore it to our VBE buffer.
    mov ax, 0x8000
    mov es, ax

    ; Restore mode number
    pop cx


    ; --------------------------------
    ; Is this mode 0x0192?
    ;
    ; 1920x1080x32 according to
    ; your VBE output.
    ; --------------------------------

    cmp cx, 0x0192
    jne .not_our_mode


    ; --------------------------------
    ; Does it support a linear
    ; framebuffer?
    ;
    ; Mode Attributes = offset 0x00
    ; Bit 7 = linear framebuffer
    ; --------------------------------

    test word [es:0x0200], 0x0080
    jz .not_our_mode


    ; --------------------------------
    ; We found mode 0192!
    ; --------------------------------

    ; Restore mode-list pointer
    pop di
    pop es


    ; --------------------------------
    ; Point ES back to VBE mode info
    ; --------------------------------

    mov ax, 0x8000
    mov es, ax


    ; --------------------------------
    ; Save framebuffer information
    ; for the kernel
    ;
    ; ES = 0x8000
    ;
    ; ES:0400 = 0x80400
    ; --------------------------------

    ; Framebuffer physical address
    mov eax, [es:0x0228]
    mov [es:0x0400], eax

    ; Pitch (bytes per scanline)
    mov ax, [es:0x0210]
    mov [es:0x0404], ax

    ; Width
    mov ax, [es:0x0212]
    mov [es:0x0406], ax

    ; Height
    mov ax, [es:0x0214]
    mov [es:0x0408], ax

    ; Bits per pixel
    xor ax, ax
    mov al, [es:0x0219]
    mov [es:0x040A], ax


    ; --------------------------------
    ; DEBUG:
    ; Print framebuffer address
    ; --------------------------------

    mov si, fb_text
    call prints

    mov eax, [es:0x0400]
    call print_hex32

    call print_newline


    ; --------------------------------
    ; DEBUG:
    ; Print pitch
    ; --------------------------------

    mov si, pitch_text
    call prints

    mov ax, [es:0x0404]
    call print_hex16

    call print_newline


    ; --------------------------------
    ; Switch to mode 0192
    ;
    ; BX bit 14 = Linear framebuffer
    ; --------------------------------

    mov bx, 0x0192
    or bx, 0x4000

    mov ax, 0x4F02
    int 0x10

    cmp ax, 0x004F
    jne vbe_error


    ; --------------------------------
    ; Enter protected mode
    ; --------------------------------

    jmp protected_mode_start


; ------------------------------------
; Current mode wasn't 0192
; ------------------------------------

.not_our_mode:

    pop di
    pop es

    add di, 2
    jmp .next_mode


; ------------------------------------
; BIOS couldn't provide information
; about this mode
; ------------------------------------

.mode_failed:

    pop cx
    pop di
    pop es

    add di, 2
    jmp .next_mode


; ------------------------------------
; Mode 0192 wasn't found
; ------------------------------------

vbe_no_mode:

    jmp $


; ------------------------------------
; VBE error
; ------------------------------------

vbe_error:

    jmp $


; ------------------------------------
; Disk error
; ------------------------------------

disk_error:

    jmp $


; ------------------------------------
; Enter protected mode
; ------------------------------------

protected_mode_start:

    cli

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode


; ------------------------------------
; GDT
; ------------------------------------

gdt_start:

    ; 0x00 null
    dq 0x0000000000000000

    ; 0x08 kernel code ring 0
    dq 0x00CF9A000000FFFF

    ; 0x10 kernel data ring 0
    dq 0x00CF92000000FFFF

    ; 0x18 user code ring 3
    dq 0x00CFFA000000FFFF

    ; 0x20 user data ring 3
    dq 0x00CFF2000000FFFF

gdt_end:


gdt_descriptor:

    dw gdt_end - gdt_start - 1
    dd gdt_start


; ------------------------------------
; Protected mode
; ------------------------------------

bits 32

protected_mode:

    mov ax, 0x10

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000

    ; Jump to kernel
    jmp 0x10000


; ------------------------------------
; Back to 16-bit
; ------------------------------------

bits 16


; ------------------------------------
; Print zero-terminated string
;
; SI = string address
; ------------------------------------

prints:

    mov ah, 0x0E

.loop:

    mov al, [si]

    cmp al, 0
    je .done

    int 0x10

    inc si

    jmp .loop

.done:

    ret


; ------------------------------------
; Print AX as 4 hexadecimal digits
;
; AX = 0x1234
; prints:
;
; 1234
; ------------------------------------

print_hex16:

    push ax
    push bx
    push cx
    push dx

    mov bx, ax
    mov cx, 4

.next:

    rol bx, 4

    mov al, bl
    and al, 0x0F

    cmp al, 9
    jbe .number

    add al, 'A' - 10
    jmp .print

.number:

    add al, '0'

.print:

    mov ah, 0x0E
    int 0x10

    loop .next

    pop dx
    pop cx
    pop bx
    pop ax

    ret


; ------------------------------------
; Print EAX as 8 hexadecimal digits
;
; EAX = 0x12345678
; prints:
;
; 12345678
; ------------------------------------

print_hex32:

    push eax
    push ebx
    push cx
    push dx

    mov ebx, eax
    mov cx, 8

.next:

    rol ebx, 4

    mov al, bl
    and al, 0x0F

    cmp al, 9
    jbe .number

    add al, 'A' - 10
    jmp .print

.number:

    add al, '0'

.print:

    mov ah, 0x0E
    int 0x10

    loop .next

    pop dx
    pop cx
    pop ebx
    pop eax

    ret


; ------------------------------------
; Print newline
; ------------------------------------

print_newline:

    mov ah, 0x0E
    mov al, 13
    int 0x10

    mov al, 10
    int 0x10

    ret


; ------------------------------------
; Debug strings
; ------------------------------------

fb_text:

    db "FB: 0x", 0


pitch_text:

    db "PITCH: 0x", 0


; ------------------------------------
; Pad boot2 to exactly 2 sectors
; ------------------------------------

times 1024-($-$$) db 0