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

#include "Thread.hpp"
#include "PIC.hpp"
#include <klib.h>
#include "portio.h"
#include "util.hpp"
#include <stdint.h>
#include "e9dump.hpp"
#include <string.h>

namespace{
    class ThreadSceduler{

    };

    ThreadList all_threads;
    ThreadList::Iterator currentThread;
}

typedef uint32_t size_t;

void EIPHack(arch::Context * state){
    
    //SUper hack to save the cs:EIP values when we task switch
    *((uint32_t*)((uint8_t*)state->esp + 8)) = state->eip;
    *((uint32_t*)((uint8_t*)state->esp + 12)) = state->cs;
    
    *((uint32_t*)((uint8_t*)state->esp + 16)) = state->eflags;

}
bool firstSwitch= false;
//This gets installed as our interrupt handler
void TaskSwitchIRQ(arch::Context *state){

    uint32_t old_eip = state->eip;
    
    e9_str("\n IRQ\nESP=");
    e9_dump(state->esp);
    outb(0xe9, '\n');
    e9_dump(currentThread->id);
    outb(0xe9, '\n');


    // Save state of current thread
    if(firstSwitch)
        currentThread->state = *state;
    else 
        firstSwitch = true;

    // Get the next thread to be run
    // THis is boring because it is just round robin scheduling
    ++currentThread;
    e9_dump(currentThread->id);
    outb(0xe9, '\n');

    
    // Write the new threads state into the interrupt state
    // This means we shout iret to it when we return from the interrupt handler
    *state = currentThread->state;

    EIPHack(state);
    
    e9_dump(old_eip);
    e9_str("->");
    e9_dump(state->eip);
    outb(0xe9, '\n');
    PIC::sendEOI(0x0);
    return;
}

Thread * Scheduler::InitThread(void (*entry)()){
    Thread* thread = (Thread*)malloc(sizeof(Thread));
    e9_str("Init Thread ");
    e9_dump((uint32_t) entry);
    outb(0xe9, '\n');
    memset(thread, 0, sizeof(Thread));
    thread->id = 0xDEADBEEf;
    thread->state.eip = (uint32_t) entry;
    //TODO hard coding the segment selectors is a major hack
    thread->state.cs = 0x08;
    // apparently interrupt handling only saves the code segment. Seems like a big oversight
    /*thread->state.ds = 0x10;
    thread->state.es = 0x10;
    thread->state.ss*/

    // create a stack for our new thread
    const uint32_t STACK_SIZE = 8 * 1024;


    //TODO: do we actually need to subtract 4? I need to think about it
    thread->state.esp = (uint32_t)malloc(STACK_SIZE) + STACK_SIZE - 8;
    e9_str("New stack =");
    e9_dump(thread->state.esp);
    outb(0xe9, '\n');
    thread->state.eflags = 0x200;
    all_threads.insert_tail(thread);
    //TODO this is just nasty
    if(*currentThread == NULL){
        currentThread = all_threads.head();
    }
    return thread;
}

// Dummy func to burn cpu cycles.
// SInce its in a different compilation unit the optimizer shouldnt be able 
// To see through it (unless you have LTO)
void DummyFunc(){

}
