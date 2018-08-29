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

#ifndef INTERUPTMANAGER_HPP
#define INTERUPTMANAGER_HPP

#include "registers.h"
#include "idt.h"
//note this matches layout of "PUSHAD" func
//TODO: save segment register state when that is an issue
typedef struct InterruptState {
  
  //uint32_t errCode;
  //uint32_t intNum;
  
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  
  uint32_t intNum;
  uint32_t errCode;
  
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
} InterruptState;


//typedef  void (*InterruptHandler)(int intNum, Registers &state);
typedef  void (*InterruptHandler)(InterruptState *state);
//Singleton
//TODO: at the moment this is x86 specific, later should be separated out
class InterruptManager {
  public:

  InterruptHandler osHandlers[256];
 // virtual bool registerInterruptHandler(int intNum, InterruptHandler handler) = 0;
  
  //protected:
  InterruptManager(){}
  
  //TODO: more elegant solution

  //virtual ~InterruptManager();
   void Initialize();
   void registerHandler(InterruptHandler handler, uint8_t vector){
     //TODO we should check if we are overwriting an existing handler
     osHandlers[vector] = handler;
   }
     
  private:
    IDT_entry idt[256];
    int dummyField;
    
    void setIDTVector(uint8_t vec, uint16_t cs, void *isr, uint8_t dpl, uint8_t type);
    void interrupt(InterruptState&);
  
};

#endif
