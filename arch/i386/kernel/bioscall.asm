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

use32

%include "/registers.mac"


extern protToReal
extern realToProt

segment .lowtext

global callBios
;void callBios(int callnum, Registers *registers)
callBios:
    ;Stack layout
    ;registers   - EBP + 12
    ;callnum     - EBP + 8
    ;ret         - EBP + 4
    ;old ebp     - EBP

    enter 0, 0
    ;Caller perserves EAX, ECX, EDX , we need to save the rest

    push ebx
    push esi
    push edi

    mov ebx, [ebp + 12] ;registers


    mov eax, [ebx + Registers.eax]
    mov [.eaxval], eax

    mov ax, [ebx + Registers.ds]
    mov [.dsval], ax
    
    mov ax, [ebx + Registers.es]
    mov [.esval], ax

    mov eax, [ebp + 8] ;callnum
    mov [.intnum], al

    mov ecx, [ebx + Registers.ecx]
    mov edx, [ebx + Registers.edx]
    xor esi, esi ;clear top 16 bits
    xor edi, edi
    mov si, [ebx + Registers.si]
    mov di, [ebx + Registers.di]
    mov ebx, [ebx + Registers.ebx]

    ;TODO: double check about invalidating instruction cache

    call protToReal
    USE16
    ;this is self modifying code

    ;mov ax, imm16
        db 0xb8
        .esval: dw 0
    mov es, ax
    ;mov ax, imm16
        db 0xb8
        .dsval: dw 0
    mov ds, ax

    ;mov eax, imm16
        db 0x66, 0xb8
        .eaxval: dd 0

    ;int imm8
        db 0xcd
        .intnum: db 0

    push eax
    xor ax,ax
    mov ds, ax
    pop eax

    mov [.eaxval], eax
    pushf
    pop ax
    mov [.esval], ax ;save flags register in esval

    call dword realToProt
    USE32

    push ebx 
    mov ebx, [ebp + 12] ;pointer to regs

    mov eax, [.eaxval]
    mov [ebx + Registers.eax], eax
    pop eax ;the old ebx value
    mov [ebx + Registers.ebx], eax
    mov [ebx + Registers.ecx], ecx
    mov [ebx + Registers.edx], edx

    mov ax, [.esval] ;we saved the flags register in esval
    mov [ebx + Registers.flags], ax

    pop edi
    pop esi
    pop ebx
    leave
    ret

