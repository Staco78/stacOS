[bits 16]
section .apBoot
global apBootStart
apBootStart:
cli
cld

mov eax, cr4
or eax, 1 << 5  ; Set PAE bit
mov cr4, eax

mov eax, dword [apCr3]
mov cr3, eax

mov ecx, 0xC0000080 
rdmsr               
or eax, 1 << 8
wrmsr

mov eax, cr0
or eax, 0x80000001 
mov cr0, eax

lgdt [GDT64.Pointer]
jmp GDT64.Code:entry64bits


[bits 64]
entry64bits:
mov ax, 0x10
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
mov rsp, [apStack]
mov rbp, rsp

extern apEntry
call apEntry

global apCr3
apCr3 dq 0

global apStack
apStack dq 0


; Access bits
PRESENT        equ 1 << 7
NOT_SYS        equ 1 << 4
EXEC           equ 1 << 3
DC             equ 1 << 2
RW             equ 1 << 1
ACCESSED       equ 1 << 0
 
; Flags bits
GRAN_4K       equ 1 << 7
SZ_32         equ 1 << 6
LONG_MODE     equ 1 << 5

GDT64:
    .Null: equ $ - GDT64
        dq 0
    .Code: equ $ - GDT64
        dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
        db 0                                        ; Base (mid, bits 16-23)
        db PRESENT | NOT_SYS | EXEC | RW            ; Access
        db GRAN_4K | LONG_MODE | 0xF                ; Flags & Limit (high, bits 16-19)
        db 0                                        ; Base (high, bits 24-31)
    .Data: equ $ - GDT64
        dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
        db 0                                        ; Base (mid, bits 16-23)
        db PRESENT | NOT_SYS | RW                   ; Access
        db GRAN_4K | SZ_32 | 0xF                    ; Flags & Limit (high, bits 16-19)
        db 0                                        ; Base (high, bits 24-31)
    .Pointer:
        dw $ - GDT64 - 1
    .Address: dq GDT64


global apBootEnd
apBootEnd:

