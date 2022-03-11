[bits 64]
section .text


%macro push_all_regs 0
push r15
push r14
push r13
push r12
push r11
push r10
push r9
push r8
push rdx
push rcx
push rbx
push rax
push rbp
push rsi
push rdi
%endmacro

%macro pop_all_regs 0
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop rdx
pop rcx
pop rbx
pop rax
pop rbp
pop rsi
pop rdi
%endmacro

global idleTask
idleTask:
sti
_:
hlt
jmp _


global _schedulerTickHandler
_schedulerTickHandler
push_all_regs
extern schedulerTickHandler
call schedulerTickHandler



; rdi = rsp, rsi = cr3
global doSwitch
doSwitch:
mov rsp, rdi
test rsi, rsi
jz not_change_cr3
mov cr3, rsi

not_change_cr3:
pop_all_regs
iretq