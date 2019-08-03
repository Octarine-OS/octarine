;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (c) 2019, Devin Nakamura
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;
; 1. Redistributions of source code must retain the above copyright notice, this
;    list of conditions and the following disclaimer.
; 2. Redistributions in binary form must reproduce the above copyright notice,
;    this list of conditions and the following disclaimer in the documentation
;    and/or other materials provided with the distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
; SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
; CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
; OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;
; SPDX-License-Identifier: BSD-2-Clause
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Various constants
PG_PRESENT  equ 1 << 0
PG_RW       equ 1 << 1
PG_USER     equ 1 << 2
PG_WT       equ 1 << 3 ; write through
PG_SZ       equ 1 << 7 ; size bit

;ie ignore last 12 bits
PAGE_ADDR_MASK equ 0xFFC00000
BOOTSTRAP_PAGE_SIZE equ 1024 * 1024 * 4
; Defines a page directory entry for a 4mb page at a given addr
; Assigns basic flags (ie, present, system pages, write through cache)
%define PAGEDIR_ENT(addr) dd (addr & PAGE_ADDR_MASK) | PG_PRESENT | PG_RW | PG_USER | PG_WT | PG_SZ

KERNEL_VIRTUAL_BASE equ 0xC0000000
BOOT_STACK_SIZE equ 0x2000

SEG_KERN_CODE equ 1 << 3
SEG_KERN_DATA equ 2 << 3

[Bits 32]
;[org 0x8000]

extern kmain
extern start_ctors
extern end_ctors
extern start_dtors
extern end_dtors

global _multibootEntry


section .text

_multibootEntry:
    cli

    ;first we want to save the bootdrive
    mov [mbinfo - KERNEL_VIRTUAL_BASE], ebx
    mov [mbmagic - KERNEL_VIRTUAL_BASE], eax

    ;Install our own GDT
    lgdt [GDT - KERNEL_VIRTUAL_BASE]
    mov AX, SEG_KERN_DATA
    mov DS, AX
    mov ES, AX
    mov FS, AX
    mov GS, AX
    mov SS, AX

    jmp SEG_KERN_CODE:.update_cs-KERNEL_VIRTUAL_BASE
    .update_cs:

    ; set up paging
    mov eax, (bootstrap_page_dir - KERNEL_VIRTUAL_BASE) ; Get physical addr of the page dir
    mov cr3, eax  ; set page dir register

    ; set the PSE bit to enable 4 mb pages
    mov eax, cr4
    or eax, 0x10
    mov cr4, eax

    ;set pg bit to enable paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ;change Jump into higher half
    ; note we indirect jump to force an absolute jump intruction
    mov eax, .higher_half
    jmp eax

.higher_half:
    ; set up our stack
    mov esp, boot_stack + BOOT_STACK_SIZE

    ;start calling constructors for global objects
    mov ebx, start_ctors
    jmp .check_ctors

    .call_constructor:
        call [EBX]
        add ebx, 4
    .check_ctors:
        cmp ebx, end_ctors
        jb .call_constructor ;if we arent at the end, keep calling

    ;Ok all our consturctors are called
    ;now lets enter the main kernel procedure
    mov eax, [mbinfo]
    push eax
    mov eax, [mbmagic]
    push eax
    ;TODO: check if I can push [mbinfo]
    call kmain


    ;the kernel has returned, that isnt supposed to have happened
    ;TODO: printing an error messasge might be usefull
    call call_destructors
    cli
    .hang:
        hlt
        jmp .hang


call_destructors:
    mov eax, start_dtors
    jmp .check_dtors

    .call_destructor:
        call [EAX]
        add eax, 4
    .check_dtors:
        cmp eax, end_dtors
        jb .call_destructor

    ret

section .bss

    mbinfo resb 4
    mbmagic resb 4

    boot_stack: resb BOOT_STACK_SIZE

section .data
%include "/gdt.mac"



; Our page directory for bootstrap
; We use 4MB pages for compactness
; 0 - KERNEL_VIRTUAL_BASE is identity mapped
; KERNEL_VIRTUAL_BASE  - 4GB is mapped to 0 - (4GB - VIRTUAL_KERNEL_BASE)
align 0x1000
bootstrap_page_dir:
%assign pg_ctr 0
; Fill in the base identity mapped pages
%rep KERNEL_VIRTUAL_BASE / BOOTSTRAP_PAGE_SIZE
    PAGEDIR_ENT(pg_ctr << 22)
    %assign pg_ctr pg_ctr+1
%endrep

; Fill in the high mapped pages
%assign high_pg_ctr 0
%rep 1024 - pg_ctr
    PAGEDIR_ENT(high_pg_ctr << 22)
    %assign high_pg_ctr high_pg_ctr+1
%endrep
