;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (c) 2018, Devin Nakamura
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

REAL_MODE_DATA_SEG equ 4 << 3
REAL_MODE_CODE_SEG equ 3 << 3

REAL_MODE_STACK_BASE equ 0x2000
KERNEL_VIRTUAL_BASE equ 0xC0000000


SEGMENT .lowtext

global protToReal

protToReal:
    ;get return address and put on real mode stack
    pop DWORD [REAL_MODE_STACK_BASE - 4]
    push ebp

    sidt [prot_idt]
    o16 lidt [real_idt]
    
    mov eax, cr3
    mov [saved_page_dir], eax

    ;save our position on the stack
    mov eax, esp
    mov [savedSP], eax


    ;set segment registers
    mov ax, REAL_MODE_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax



    ;set up real mode stack
    mov eax, REAL_MODE_STACK_BASE - 4
    mov esp, eax
    mov ebp, eax

    jmp REAL_MODE_CODE_SEG:.temp_real_code

.temp_real_code:
    [bits 16]
    ;clear p-mode and paging flags
    mov eax, cr0
    and eax, 0x7FFFFFFF ; disable paging bit
    mov cr0, eax
    and al, 0xFE ;PE flag is bit 0
    mov cr0, eax
    jmp  0:.real_mode

.real_mode:
    ;reload all the segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    sti ;we want to re-enable interrupts
    
    o32 ret

global realToProt
realToProt:
    cli
    ; Note lgdt [X] is implictly lgdt [DS:x]
    xor ax, ax
    mov ds, ax
    lgdt [GDT - KERNEL_VIRTUAL_BASE]

    ;Load the page directory
    mov eax, [saved_page_dir]
    mov cr3, eax

    mov eax, cr0
    or al, 1 ;set p-mode bit
    mov cr0, eax

    ;we need to replace the CS descriptor cached value with one from
    ;the GDT
    jmp  08h:.pmode_start
.pmode_start:
    [Bits 32]

    ;enable paging
    mov eax, cr0
    or eax, 0x80000000 ; set paging bit
    mov cr0, eax

    ;lets update all the data segment descriptors
    mov AX, 0x10 ;ie the 3rd gdt entry
    mov DS, AX
    mov ES, AX
    mov FS, AX
    mov GS, AX
    mov SS, AX
    
    ;restore idt
    lidt [prot_idt]

    ;grab the return address before we wipe it out
    pop WORD [REAL_MODE_STACK_BASE -2]
    mov eax, [savedSP]
    mov esp, eax
    pop ebp
    
    
    ;put the return address on the new stack
    xor eax, eax
    mov ax, [REAL_MODE_STACK_BASE - 2]
    push eax
o16    ret

section .lowdata
saved_page_dir: dd 0
real_idt:
    dw 0x3FF
    dd 0

%include "/gdt.mac"

prot_idt:
    dw 0
    dd 0
	
;segment .bss

saved_idt resq 1
realidt resq 1
savedSP resq 1

