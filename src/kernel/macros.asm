%macro push_all_regs 0
push rax
push rbx
push rcx
push rdx
push rsi
push rdi
push rbp
push r8
push r9
push r10
push r12
push r13
push r11
push r14
push r15
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
pop rbp
pop rdi
pop rsi
pop rdx
pop rcx
pop rbx
pop rax
%endmacro


%macro swapgs_if_necessary 0-1 8
test byte [rsp + %1], 3
jz $+5
swapgs
%endmacro