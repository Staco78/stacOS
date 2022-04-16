%include "macros.asm"

[bits 64]
section .text

extern interruptsHandler


%macro ISR_COMMON 1
swapgs_if_necessary 16
push_all_regs
mov rdi, %1
mov rsi, rsp
call interruptsHandler
pop_all_regs
add rsp, 8
swapgs_if_necessary
iretq
%endmacro

%macro IRQ 2
global irq_%1
irq_%1:
    cli
    push qword 0
    ISR_COMMON %2
%endmacro

%macro ISR_ERROR 1
global isr_%1
isr_%1:
    cli
    ISR_COMMON %1
%endmacro

%macro ISR_NO_ERROR 1
global isr_%1
isr_%1:
    cli
    push qword 0
    ISR_COMMON %1
%endmacro


IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

ISR_NO_ERROR  0
ISR_NO_ERROR  1
ISR_NO_ERROR  2
ISR_NO_ERROR  3
ISR_NO_ERROR  4
ISR_NO_ERROR  5
ISR_NO_ERROR  6
ISR_NO_ERROR  7
ISR_ERROR 8
ISR_NO_ERROR  9
ISR_ERROR 10
ISR_ERROR 11
ISR_ERROR 12
ISR_ERROR 13
ISR_ERROR 14
ISR_NO_ERROR  15
ISR_NO_ERROR  16
ISR_ERROR  17
ISR_NO_ERROR  18
ISR_NO_ERROR 19
ISR_NO_ERROR 20
ISR_NO_ERROR 21
ISR_NO_ERROR 22
ISR_NO_ERROR 23
ISR_NO_ERROR 24
ISR_NO_ERROR 25
ISR_NO_ERROR 26
ISR_NO_ERROR 27
ISR_NO_ERROR 28
ISR_NO_ERROR 29
ISR_ERROR 30
ISR_NO_ERROR 31
