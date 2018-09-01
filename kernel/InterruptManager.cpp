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

#include "InterruptManager.hpp"
#include "terminal.h"
#include "portio.h"
#include "util.hpp"
void InterruptManager::setIDTVector(uint8_t vec, uint16_t cs, void *isr, uint8_t dpl, uint8_t type){
    idt[vec].offset_low = ((uint32_t)isr) & 0xFFFF;
    idt[vec].selector = cs;
    idt[vec].zero = 0;
    
    idt[vec].type_attr = 0x80 | ((dpl & 0x3)<<5) | (type & 0xF);
    idt[vec].offset_high = (((uint32_t)isr) >> 16) & 0xFFFF;
    
}
extern "C" void* isr_table[];
extern "C" void* intMgrInstance;
void InterruptManager::Initialize(){
    static struct {
        uint16_t limit;
        void *idt_addr;
        
    } __attribute__((packed)) idtr;
    for(int i=0; i < 256; ++i){
    //TODO fix cs val
    //TODO fix type val
        setIDTVector((uint8_t)i, 8, isr_table[i], 0, 0x0E);
        osHandlers[i] = nullptr;
    }
    
    intMgrInstance = this;
    dummyField = 0xDEADBEEF;
    idtr.limit=sizeof(idt)-1;
    idtr.idt_addr = idt;
    asm volatile("lidt %0" : : "m"(idtr));

}

static void dumpRegister(int line, int col, const char * name, uint32_t val){
    Terminal &term = *globalTerm;
    char buff[9];
    buff[8] = 0;
    const int colNum = 1 + (17 * col);

    term.setCursor(colNum, line);

    term.printString(name);
    term.printString(": ");
    shittyHexStr((val >> 16) & 0xFFFF, buff);
    term.printString(buff);
    term.printChar(' ');
    shittyHexStr(val & 0xFFFF, buff);
    term.printString(buff);
}

void testPanic(){
    //Clear screen
    Terminal &term = *globalTerm;
    
    term.setAttr(0x47);
    term.clearScreen();
    term.setCursor(1, 0);
    term.printString("KERNEL PANIC TEST!");
    term.setCursor(1, 1);
    term.printString("Explanatory message here.");
    
    dumpRegister(10, 0, "EAX", 0x12345678);
    dumpRegister(10, 1, "ECX", 0xDEADBEEF);
    
    dumpRegister(11, 0, "EDX", 0x87654321);
    dumpRegister(11, 1, "EBX", 0xB00BB00B);
    
    dumpRegister(12, 0, "ESP", 0xABCDEF0);
    dumpRegister(12, 1, "EBP", 0xFFFFFFF);
    
    dumpRegister(13, 0, "ESI", 0x8C8C8C8);
    dumpRegister(13, 1, "EDI", 0x0000000);
    
    dumpRegister(14, 0, "EIP", 0x55448899);
}


const char *exceptionNames[] = {
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
  

void exceptionPanic(InterruptState &state){
    //Clear screen
    asm("cli");
    Terminal &term = *globalTerm;
    
    term.setAttr(0x47);
    term.clearScreen();
    term.setCursor(1, 0);
    term.printString("KERNEL PANIC: Unhandled Exception");
    term.setCursor(1, 1);
    if(state.intNum <= 0x1f) {
      term.printString(exceptionNames[state.intNum]);

    }
          char buffer[9];
      shittyHexStr32(state.intNum, buffer);
      buffer[9] = 0;
      term.setCursor(1,2);
      term.printString("Exception 0x");
      term.printString(buffer);
      term.setCursor(26,2);
      term.printString("Code: ");
      shittyHexStr32(state.errCode, buffer);
      term.printString(buffer);
    //term.printString("Explanatory message here.");
    
    dumpRegister(10, 0, "EAX", state.eax);
    dumpRegister(10, 1, "ECX", state.ecx);
    
    dumpRegister(11, 0, "EDX", state.edx);
    dumpRegister(11, 1, "EBX", state.ebx);
    
    dumpRegister(12, 0, "ESP", state.esp);
    dumpRegister(12, 1, "EBP", state.ebp);
    
    dumpRegister(13, 0, "ESI", state.esi);
    dumpRegister(13, 1, "EDI", state.edi);
    
    dumpRegister(14, 0, "EIP", state.eip);
    while(true){}
    //dumpRegister(14, 0, "EIP", state.eip);
    
}

/*    term.printString("Boot drive: ");
    char buff[32];
    ucvt(bootdrive, buff, 10, 3, 0);
    term.printString(buff);*/
    

void InterruptManager::interrupt(InterruptState& state){
  if (state.intNum <= 0x1F){
    /*globalTerm->printString("ERROR: Unrecovverable exception.\n");
    
    char buff[9];
    globalTerm->printString("Got Interrupt: 0x");
    
    shittyHexStr32(state.intNum, buff);
    buff[8] = 0;
    globalTerm->printString(buff);
    while(1){};*/
    exceptionPanic(state);
    //globalTerm->newLine();
    //shittyHexStr32(dummyField, buff);
    //globalTerm->printString(buff);
  }
  if(state.intNum >=256){
    globalTerm->printString("ERROR: bad interrupt number: ");
    char buff[9];
    shittyHexStr32(state.intNum, buff);
    buff[8] = 0;
    globalTerm->printString(buff);
    while(true){};
  }
  //buff[]=0;,
  if(nullptr == osHandlers[state.intNum]){
    globalTerm->printString("ERROR: No OS handler installed!\n");
    char buff[9];
    shittyHexStr32(state.intNum, buff);
    buff[8] = 0;
    globalTerm->printString(buff);
    outb(0xe9, 'X');
    while(true){}
    outb(0xe9, '!');
  }
  
  osHandlers[state.intNum](&state);
  

  asm("sti");
  //ucvt(state.intNum, buff, 10, 3, 0);
  //globalTerm->printString(buff);
  
}
