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

extern _ZN4i3863IDT9InterruptERNS_7ContextE
global isr_table
global intMgrInstance
global isr32
global interrupt_handler_core
%macro DEFINE_ISR 1
  isr%1:
 
    ;add dummy error code
    push dword 0
    ; Push the interrupt number
    push dword %1
 
    jmp interrupt_handler_core
%endmacro
%macro DEFINE_ISR_ERR 1
  isr%1:
    ; Push the interrupt number
    push dword %1
 
    jmp interrupt_handler_core
%endmacro

segment .text

;Core routine which handles interrupts
interrupt_handler_core:
  ;save all regs
  pushad
  
  ;new stack frame
  xor ebp,ebp
  
  ;Get the address of our state struct (ie result of pushad)
  mov eax, esp
  ;add eax, 4 ;note super hack
  push eax
  call _ZN4i3863IDT9InterruptERNS_7ContextE
  
  ;clean up stack
  add esp, 8 
  
  ;restore old state
  popad

  ;Note popad doesnt restore ESP
  mov esp, [esp - 20];
  
  ;interrupt added errcode and int number to stack
  add esp, 0x08
  sti 
  iret


DEFINE_ISR 0
DEFINE_ISR 1
DEFINE_ISR 2
DEFINE_ISR 3
DEFINE_ISR 4
DEFINE_ISR 5
DEFINE_ISR 6
DEFINE_ISR 7
DEFINE_ISR_ERR 8
DEFINE_ISR 9
DEFINE_ISR_ERR 10
DEFINE_ISR_ERR 11
DEFINE_ISR_ERR 12
DEFINE_ISR_ERR 13
DEFINE_ISR_ERR 14
DEFINE_ISR 15
DEFINE_ISR 16
DEFINE_ISR_ERR 17
%assign i 18
%rep 256 - 18
DEFINE_ISR i
%assign i i+1
%endrep
 
 
segment .data
isr_table:
%assign i 0
%rep 256
dd isr %+ i
%assign i i+1
%endrep
