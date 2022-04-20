[bits 64]
section .rodata
txt db "Hello World", 10
length equ $ - txt

section .text
global _start
_start:
    mov rax, 1
    mov rdi, 0
    mov rsi, txt
    mov rdx, length
    syscall
    xor rax, rax
    mov rdi, 0
    syscall