[org 0x7c00]

jmp diskload

; mov ah, 0x0e        bios teletype mode
; mov al, '(letter)'  letter to print into al
; int 0x10            bios interrupt 0x10
; Move string to si register to print
prints:
    mov ah, 0x0e

.loop:
    mov al, [si]

    cmp al, 0
    je .done

    int 0x10

    inc si
    jmp .loop

.done:
    ret

diskload:
    xor ax, ax
    mov ds, ax
    mov es, ax

    mov bx, 0x7e00

    ;boot drive saved at 0x0500
    mov [0x0500], dl
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [0x0500]

    int 0x13
    jc disk_err

    jmp 0x0000:0x7e00

disk_err:
    mov si, disk_err_msg
    call prints

    jmp $

disk_err_msg:
    db "disk err", 0

; No data past this point!
exit:
    jmp $

times 510-($-$$) db 0
db 0x55, 0xaa

;
; Sector 2 from 0x8000
;
