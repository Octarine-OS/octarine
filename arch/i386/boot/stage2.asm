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

global _start

section .text

_start:
    ;first we want to save the bootdrive
    mov [bootdrive], dl

    ;start calling constructors for global objects
    mov ebx, start_ctors
    jmp .check_ctors

    .call_constructor:
        call [EBX]
        add ebx, 4
    .check_ctors:
        cmp ebx, end_ctors
        jb .call_constructor ;if we aren't at the end, keep calling

    ;Ok all our constructors are called
    ;now lets enter the main kernel procedure
    xor edx, edx
    mov dl, [bootdrive]
    push edx
    call kmain


    ;the kernel has returned, that isn't supposed to have happened
    ;TODO: printing an error message might be useful
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
    bootdrive resb 1
