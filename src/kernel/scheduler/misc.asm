[bits 64]
section .text

global getCurrentCPU
getCurrentCPU:
mov qword rax, [gs:00]
ret