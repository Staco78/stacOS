[bits 64]
section .text

extern log
txt db "invalid", 0
vld db "valid", 0

callSelected:
cmp rax, MAX_SYSCALL
jg notFound
mov rcx, r10
call [table_start + rax * 8]
ret
notFound:
mov rax, -88 ; ENOSYS
ret

global syscall_entry
syscall_entry:
    swapgs

    mov dword [gs:16], 0 ; lockLevel

    mov [gs:8], rsp
    mov rsp, [gs:24]

    push qword [gs:8]
    push rcx
    push r11

    sti

    call callSelected

    cli

    pop r11
    pop rcx
    pop rsp


    swapgs
    o64 sysret



section .rodata

%macro defineCall 1
extern %1
dq %1
%endmacro

table_start:
defineCall sysExit
defineCall sysOpen
defineCall sysWrite
defineCall sysRead
MAX_SYSCALL equ ($ - table_start) / 8 - 1