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

REAL_MODE_DATA_SEG equ 0
REAL_MODE_CODE_SEG equ 0

REAL_MODE_STACK_BASE equ 0


SEGMENT .TEXT

global protToReal

protToReal:
    sidt saved_idt
    mov eax, esp
	mov [savedSP], eax
	
	;set segment registers
	mov ax, REAL_MODE_DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	;get the return address and put it on the real-mode stack
	mov eax, [esp] 
	mov [REAL_MODE_STACK_BASE], eax
	
	;set up real mode stack
	mov eax, REAL_MODE_STACK_BASE
	sub eax, 4
	mov esp, eax
	mov ebp, eax
	
	jmp REAL_MODE_CODE_SEG:tmp_real_code

temp_real_code:
	[bits 16]
	;clear p-mode flag
	mov eax, cr0
	and al, 0xF7 ;PE flag is bit 0
	mov cr0, eax
	jmp 0:realmode
	
real_mode:
	;reload all the segment registers
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	
	lidt 0 ;TODO double check
	sti ;we want to re-enable interrupts
	
	o32 ret
	

segment .BSS
saved_idt resq 1
realidt resq 1
savedSP resq 1