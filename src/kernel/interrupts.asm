bits 32

global timer_interrupt
extern timer_handler

global keyboard_interrupt
extern keyboard_handler

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