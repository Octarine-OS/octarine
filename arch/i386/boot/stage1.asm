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

;this is the first stage of the boot loader
global start
global _start
;Define Contants
INITIAL_STACK_POINTER equ 0x2000
MAGIC_SIGNATURE equ 0xBEEF ;the magic signature




[Bits 16]
;[org 0x7C00] ;sets up the registers properly
[SEGMENT .text]
jmp code ;sets up the registers properly

bootdrive db 0



stage2_cylinder db 0
stage2_head db 0
stage2_sector db 2
stage2_sectors db 17 ;the size of stage 2 in sectors

retry_loop dw 4 ;the number of retrys for floppy  reads
temp dw 0 ;to hold temp values
;begin messages
booting db 'OCTARINE is now booting',0
diskerr db 'OCTARINE has encoutered a disk error code:',0
cantfind db 'OCTARINE cannot find the second stage',0
a20msg db 'A20 failure', 0

code:
    jmp 0:start ;ensure that CS  is set to 0

_start:
start:
    cli ;we dont want interrupts until we have the stack working

    xor ax, ax
    mov es, ax
    mov ds, ax
    mov ss, ax

    mov sp, INITIAL_STACK_POINTER

    sti ;ok interrupts are safe now

    ;ensure all segments point to same place
    mov ax, cs
    mov es, ax
    mov ds, ax

    mov [bootdrive], dl ;record drive that has been booted from so it isnt lost

    ;lets enable a20
    mov ax, 0x2401
    int 0x15
    jc a20_fail


    ;begin setup for reading the disk
    mov ah, 02h                 ;function 2
    mov al, [stage2_sectors]    ;number of sectors to read
    mov ch, [stage2_cylinder]   ;cylnder of addr
    mov cl, [stage2_sector]     ;sector of addr
    mov dh, [stage2_head]       ;the head of the addr
    mov dl, [bootdrive]


    ;write the data to 0x800:0x0000
    mov bx, 0x800
    mov es, bx
    xor bx, bx
    int 0x13

    jc hang ;FIXME, just hangs if there was an error

    ;ok lets get ready to make the move into protected mode
    cli
    lgdt [GDT]
    mov EAX, CR0 ;enable protected mode bit in CR0
    or AL, 1
    MOV CR0, EAX

    ;we need to replace the CS descriptor cached value with one from
    ;the GDT
    jmp 08h:pmode_start

pmode_start:
    [Bits 32]
    ;lets update all the data segment descriptors
    mov AX, 0x10 ;ie the 2nd gdt entry
    mov DS, AX
    mov ES, AX
    mov FS, AX
    mov GS, AX
    mov SS, AX

    mov esp, 0x68000

    xor edx, edx
    mov dl, [bootdrive]
    ;lets jump to the 2nd stage loader
    jmp 0x8000


    ;checksig: ;ensure valid second stage
    ;mov dx, sig ;load up the sig
    ;mov ax, bp
    ;cmp dx, ax

    ;jmp ready
    ;je ready ;if the sigs match jump the start of the code

    ;mov si, cantfind
    ;jmp hang

;ready:
;jmp 07E0h:03h
    ;mov bootdrive, dl ;record drive that has been booted from

dskerr:
mov si, diskerr
call printstring
hang:
    hlt ;save some power?
    jmp hang


;begin subroutines
;---------------------------------------------
; call when enabling a20 line has failed
;---------------------------------------------
a20_fail:
    mov si, a20msg
    call printstring
    jmp hang

;---------------------------------------------
; string needs to be null termninated
; address of string loaded into si
;---------------------------------------------
printstring: ;call to print a strint
    lodsb  ;load first byte off si (and increment)

    or al , al ;or al and al 
    jz .done ;if null char is reached skip to end

    mov ah, 0x0E ; set ah with character proerties
    int 0x10 ;call int 10

    jmp printstring ;restart process

    .done: ;si has hit a null termanator

        ret; return


;---------------------------------------------
; prints out a newline on the screen
;---------------------------------------------
newline: ;call to write a new line 


    mov AL, 0Ah ;print out end of line

    mov ah, 0x0E ; set ah with character proerties
    int 0x10 ;call int 10

    ret; return



;end subroutines

%include "/gdt.mac"

times 510-($-$$) db 0 ;pad out file

dw 0AA55h ;bootable signature
