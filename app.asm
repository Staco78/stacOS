[bits 64]
section .data
y dq 12


section .bss
x resq 1

section .text
global _start
_start:
mov rax, [x]
mov rbx, [y]