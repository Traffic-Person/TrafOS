bits 32

global timer_interrupt
global keyboard_interrupt
global syscall_interrupt

extern timer_handler
extern keyboard_handler
extern syscall_handler


timer_interrupt:
    pusha
    call timer_handler
    popa
    iretd

keyboard_interrupt:
    pusha 
    call keyboard_handler
    popa
    iretd

syscall_interrupt
    pusha
    call syscall_handler
    popa
    iretd