BITS 32

MAGIC equ 0xE85250D6
ARCH equ 0
LENGTH equ (multiboot_end - multiboot_start)
CHECKSUM equ -(MAGIC + LENGTH)

VIDEO_PTR equ 0xB8000

KERNEL_VIRTUAL_ADDR equ 0xFFFFFFFF80000000 ; -2Go
KERNEL_PML4_INDEX equ ((KERNEL_VIRTUAL_ADDR >> 39) & 0x1FF) ; 511
KERNEL_PDPT_INDEX equ ((KERNEL_VIRTUAL_ADDR >> 30) & 0x1FF) ; 510

PHYSICAL_MAPPING_ADDR equ 0xFFFF800000000000
PHYSICAL_MAPPING_PML4_INDEX equ ((PHYSICAL_MAPPING_ADDR >> 39) & 0x1FF) ; 256

section .multiboot
align 8
multiboot_start:
dd MAGIC
dd ARCH
dd LENGTH
dd CHECKSUM

align 8
dw 0
dw 0
dd 8
multiboot_end:


section .boot.data
no_64_error_msg: db "Fatal: your CPU doesn't support 64 bits"
no_64_error_msg_len equ $ - no_64_error_msg

no_1gb_pages_msg: db "Fatal: your CPU doesn't support 1GB pages"
no_1gb_pages_msg_len equ $ - no_1gb_pages_msg


align 4096
kernel_pml4:
times 512 dq 0

align 4096
kernel_PDPT:
times 512 dq 0

align 4096
kernel_PDPT2:
times 512 dq 0

align 4096
kernel_PDT:
times 512 dq 0

align 4096
physical_pml3:
times 512 dq 0

multiboot_struct dq 0



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
    ; .TSS: equ $ - GDT64
    ;     dd 0x00000068
    ;     dd 0x00CF8900
    .Pointer:
        dw $ - GDT64 - 1
    .Address: dq GDT64

section .boot.text


VIDEO_PTR equ 0xB8000
%macro panic 2
cli
mov eax, %1
mov ebx, 0
%1_loop:
mov cl, [eax + ebx]
mov [VIDEO_PTR + ebx * 2], cl
inc ebx
cmp ebx, %2
jl %1_loop

%1_loop_hlt:
hlt
jmp %1_loop_hlt
%endmacro


no_64:
panic no_64_error_msg, no_64_error_msg_len

no_1gb_pages:
panic no_1gb_pages_msg, no_1gb_pages_msg_len


global _start
_start:

cli ; disable interrupts

mov [multiboot_struct], ebx

mov eax, 0x80000000 ; CPU indentification
cpuid
cmp eax, 0x80000001
jb no_64
mov eax, 0x80000001
cpuid
test edx, 1 << 29
jz no_64
test edx, 1 << 26
jz no_1gb_pages


;setup paging
mov eax, kernel_PDPT
or eax, 3
mov [kernel_pml4], eax
mov eax, kernel_PDT
or eax, 3
mov [kernel_PDPT], eax

mov eax, 131
mov ebx, kernel_PDT
loop_PDT:
mov [ebx], eax
add eax, 0x200000
add ebx, 8
cmp ebx, kernel_PDT + 4096
jl loop_PDT

mov eax, kernel_PDPT2
or eax, 3
mov [kernel_pml4 + KERNEL_PML4_INDEX * 8], eax
mov eax, kernel_PDT
or eax, 3
mov [kernel_PDPT2 + KERNEL_PDPT_INDEX * 8], eax

mov eax, cr4
or eax, 1 << 5
mov cr4, eax

mov eax, kernel_pml4
mov cr3, eax

mov ecx, 0xC0000080
rdmsr
or eax, 1 << 8
wrmsr

mov eax, cr0
or eax, 1 << 31
mov cr0, eax

lgdt [GDT64.Pointer]
jmp GDT64.Code:entry64bits



[BITS 64]
entry64bits:
    mov rsp, stack_top
    mov rbp, rsp
    jmp entryUpper
   

section .text
entryUpper:
    add qword [GDT64.Address], 0xFFFFFFFF80000000
    lgdt [GDT64.Pointer + 0xFFFFFFFF80000000]

    ; we assume we have 1GB pages
    mov rax, 131
    mov rbx, physical_pml3
    loop_physical_pml3:
    mov [rbx], rax
    add rax, 0x40000000
    add rbx, 8
    cmp rbx, physical_pml3 + 4096
    jl loop_physical_pml3

    mov rax, physical_pml3 
    or rax, 3
    mov [kernel_pml4 + PHYSICAL_MAPPING_PML4_INDEX * 8], rax

    mov rdi, [multiboot_struct]
    add rdi, KERNEL_VIRTUAL_ADDR
    mov qword [kernel_pml4], 0

    mov rsi, kernel_pml4

    extern kernelMain
    call kernelMain
    halt:
    cli
    hlt
    jmp halt


section .bss
stack_bottom:
resb 16384
stack_top: