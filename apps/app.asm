[bits 64]
section .rodata
txt db "/file", 0
length equ $ - txt
_error db "error", 10
_error_length equ $ - _error

section .bss
buff resb 30

section .text
global _start
_start:
    mov rax, 1
    mov rdi, txt
    mov rsi, 1
    syscall

cmp rax, 0
    jl error
    
    mov rdi, rax
    mov rax, 3
    mov rsi, buff
    mov rdx, 30
    mov r10, 0
    syscall

    cmp rax, 0
    jl error

    mov rax, 2
    mov rdi, 1
    mov rsi, buff
    mov rdx, 30
    syscall

    cmp rax, 30
    jl error
    
    xor rax, rax
    mov rdi, 0
    syscall

error:

    mov rax, 2
    mov rdi, 1
    mov rsi, _error
    mov rdx, _error_length
    syscall

    xor rax, rax
    mov rdi, 0
    syscall