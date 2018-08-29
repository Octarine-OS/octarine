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
    ;first we want to save the bootdrive
    mov [mbinfo], ebx
	mov [mbmagic], eax
	
	;Install our own GDT
	lgdt [GDT]
	mov AX, 0x10 ;ie the 2nd gdt entry
    mov DS, AX
    mov ES, AX
    mov FS, AX
    mov GS, AX
    mov SS, AX

	;TODO: this is bad, we could be overwriting our multiboot info
    mov esp, 0x68000

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

section .data
%include "/gdt.mac"
