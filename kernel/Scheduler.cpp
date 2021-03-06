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

#include "PIC.hpp"
#include "Thread.hpp"
#include "e9dump.hpp"
#include "portio.h"
#include "util.hpp"
#include <Octarine.hpp>
#include <Scheduler.hpp>
#include <klib.h>
#include <klib_new.hpp>
#include <stdint.h>
#include <string.h>
#include <type_traits>

#include "e9dump.hpp"
namespace {
class ThreadSceduler {};

ThreadList all_threads;
ThreadList::Iterator currentThread;
} // namespace

typedef uint32_t size_t;

void EIPHack(arch::Context* state) {
	// SUper hack to save the cs:EIP values when we task switch
	*((uint32_t*)((uint8_t*)state->esp + 8)) = state->eip;
	*((uint32_t*)((uint8_t*)state->esp + 12)) = state->cs;
	*((uint32_t*)((uint8_t*)state->esp + 16)) = state->eflags;
}

/**
 * Initialize the scheduler
 */
void Scheduler::Init() {
	// This only makes sense if no other threads exist
	KASSERT(*currentThread == nullptr);
	// Allocate a thread which will represent the current execution context
	Thread* thread = (Thread*)malloc(sizeof(Thread));
	KASSERT(thread != nullptr);
	thread->id = 0;
	all_threads.insert_tail(thread);
	currentThread = all_threads.begin();
}

// This gets installed as our interrupt handler
void Scheduler::TaskSwitchIRQ(arch::Context* state) {
	KASSERT(*currentThread != nullptr);

	// Save state of current thread
	currentThread->state = *state;

	// Get the next thread to be run
	// This is boring because it is just round robin scheduling
	++currentThread;

	// Write the new threads state into the interrupt state
	// This means we shout iret to it when we return from the interrupt handler
	*state = currentThread->state;

	EIPHack(state);

	PIC::sendEOI(0x0);
	return;
}

static void deathFunction() {
	while (true) {
		// spin
	}
}

Thread* Scheduler::impl::_InitThread(ThreadStack stack,
                                     void (*entry)(void* arg), void* arg) {
	Thread* thread = (Thread*)malloc(sizeof(Thread));
	KASSERT(thread != nullptr);
	memset(thread, 0, sizeof(Thread));
	thread->id = 0xDEADBEEf;

	// apparently interrupt handling only saves the code segment. Seems like a
	// big oversight
	/*thread->state.ds = 0x10;
	thread->state.es = 0x10;
	thread->state.ss*/

	// Push first arg
	stack.push(arg);

	// Set up our stack frame
	// TODO: should be factored out into arch specific code
	// Add a backstop in case thread returns
	stack.push(&deathFunction);

	memset(&thread->state, 0, sizeof(arch::Context));

	thread->state.eip = (uint32_t)entry;
	// TODO hard coding the segment selectors is a major hack
	thread->state.cs = 0x08;
	thread->state.esp =
	    ((uint32_t)stack.top()) -
	    (sizeof(arch::Context) - offsetof(arch::Context, intNum));
	thread->state.eflags = 0x200;

	// Add dummy args to make it look like an interrupt frame
	stack.push(thread->state);

	thread->stack = stack;

	all_threads.insert_tail(thread);

	KASSERT(*currentThread != nullptr);
	return thread;
}

// Dummy func to burn cpu cycles.
// SInce its in a different compilation unit the optimizer shouldnt be able
// To see through it (unless you have LTO)
void DummyFunc() {}
