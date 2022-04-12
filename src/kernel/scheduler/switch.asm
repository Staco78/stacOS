%include "macros.asm"

[bits 64]
section .text

global idleTask
idleTask:
sti
_:
hlt
jmp _


global _schedulerTickHandler
_schedulerTickHandler:
push qword 0
push_all_regs
mov rdi, rsp
extern schedulerTickHandler
jmp schedulerTickHandler



; rdi = rsp, rsi = cr3
global doSwitch
doSwitch:
mov rsp, rdi
test rsi, rsi
jz not_change_cr3
mov cr3, rsi

not_change_cr3:
pop_all_regs
add rsp, 8
iretq