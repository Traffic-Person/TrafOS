[org 0x10000]
bits 32

kernel_start:
    ; VGA text mem
    mov byte [0xB8000], 'K'
    mov byte [0xB8001], 0x04

    mov byte [0xB8002], 'e'
    mov byte [0xB8003], 0x03
    
    mov byte [0xB8004], 'r'
    mov byte [0xB8005], 0x04

    mov byte [0xB8006], 'n'
    mov byte [0xB8007], 0x03

    mov byte [0xB8008], 'e'
    mov byte [0xB8009], 0x04

    mov byte [0xB800A], 'l'
    mov byte [0xB800B], 0x03

cli
hlt
jmp $