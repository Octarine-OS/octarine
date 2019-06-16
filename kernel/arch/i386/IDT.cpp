/*
 * Copyright (c) 2018, Devin Nakamura
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "IDT.hpp"
#include "IRQManager.hpp"
#include "terminal.h"
#include "util.hpp"

extern "C" void* isr_table[];

//TODO this will break when we change to higher-half kernel
i386::IDT::IDT(){
    for(int i=0; i < 256; ++i){
    //TODO fix cs val
    //TODO fix type val
        SetIDTVector((uint8_t)i, 8, isr_table[i], 0, 0x0E);
    }

    static struct {
        uint16_t limit;
        void *idt_addr;
    } __attribute__((packed)) idtr;
    idtr.limit=sizeof(idt)-1;
    idtr.idt_addr = idt; // TODO get physical address
    asm volatile("lidt %0" : : "m"(idtr));
}

void i386::IDT::Dummy(){
}

void i386::IDT::SetIDTVector(uint8_t vec, uint16_t cs, void *isr, uint8_t dpl, uint8_t type){
    idt[vec].offset_low = ((uint32_t)isr) & 0xFFFF;
    idt[vec].selector = cs;
    idt[vec].zero = 0;
    
    idt[vec].type_attr = 0x80 | ((dpl & 0x3)<<5) | (type & 0xF);
    idt[vec].offset_high = (((uint32_t)isr) >> 16) & 0xFFFF;
    
}



i386::IDT idt; 


static const char *exceptionNames[] = {
  "Divide by zero error",
  "Debug",
  "Non-maskable Interrupt",
  "Breakpoint",
  "Overflow",
  "Bound Range Exceeded",
  "Invalid Opcode",
  "Device Not Available",
  "Double Fault",
  "Co-Processor Segment Overrun",
  "Invalid TSS",
  "Stack-Segment Fault",
  "Segment Not Present",
  "General Protection Fault",
  "Page Fault",
  "Reserved (0x0F)",
  "x87 Floating-Point Exception",
  "Alignment Check",
  "Machine Check",
  "SIMD Floating-Point Exception",
  "Virtualization Exception",
  "Reserved (0x15)",
  "Reserved (0x16)",
  "Reserved (0x17)",
  "Reserved (0x18)",
  "Reserved (0x19)",
  "Reserved (0x1A)",
  "Reserved (0x1B)",
  "Reserved (0x1C)",
  "Reserved (0x1D)",
  "Security Exception",
  "Reserved (0x1F)"
};

static void ExceptionPanic(arch::Context &ctx) {
    //TODO implement
    while(1){}
}

void i386::IDT::Interrupt(arch::Context &ctx){
  if (ctx.intNum <= 0x1F){
    ExceptionPanic(ctx);
  }
  if(ctx.intNum >=256){
    globalTerm->printString("ERROR: bad interrupt number: ");
    char buff[9];
    shittyHexStr32(ctx.intNum, buff);
    buff[8] = 0;
    globalTerm->printString(buff);
    while(true){};
  }
  IRQManager::DoIRQ(ctx.intNum - 0x20);
}
