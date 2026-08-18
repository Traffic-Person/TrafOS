bits 32 

global _start
global enter_user_mode
extern kernel_main

section .text

_start:
    cli

    mov esp, 0x90000

    call kernel_main

.hang:
    cli
    hlt
    jmp .hang

enter_user_mode:
    cli

    ; user data
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23
    push 0xA0000

    pushfd
    pop eax
    and eax, 0xFFFFDFF
    push eax

    push 0x1B
    push user_test

    iret

user_test:
    ;exec in ring 3
    mov edi, 0xB8000

    mov word [edi], 0x0F55
    mov word [edi + 2], 0x0F53
    mov word [edi + 4], 0x0F45
    mov word [edi + 6], 0x0F52

.user_hang:
    jmp .user_hang