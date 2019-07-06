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

#ifndef THREAD_HPP
#define THREAD_HPP

#include "List.hpp"
#include "machine/Context.hpp"
#include <stddef.h>
#include <utility.hpp>

class ThreadStack {
	void* _base;
	void* _top;

  public:
	ThreadStack(ThreadStack&) = delete;
	ThreadStack(ThreadStack&& other);

	template <typename T> void push(const T& value) {
		// TODO: should we be doing some checks on T?
		size_t size = sizeof(T);
		// Allocate size for object, and round to 8 byte boundry
		_top = reinterpret_cast<void*>(
		    (reinterpret_cast<uintptr_t>(_top) - size) & ~7);
		new (_top) T(value);
	}

	void* top() { return _top; }

	static ThreadStack make(size_t size);

  private:
	ThreadStack(void* addr);
	~ThreadStack();
};

struct Thread {
	int id;
	arch::Context state; // TODO rename to context?
	ListHook<Thread> _listHook;
	ThreadStack stack;
};

typedef List<Thread, &Thread::_listHook> ThreadList;
void TaskSwitchIRQ(arch::Context* state);

namespace Scheduler {
void Init();
//Thread* InitThread(void (*entry)());

//private:
Thread* _InitThread(ThreadStack stack);

template<typename T>
Thread* InitThread(const T& runnable){
    // TODO parameterize stack size
    ThreadStack stack = ThreadStack::make(2048);
    stack.push(runnable);
    _InitThread(std::move(stack));
}
}
// TODO this is all horibly intolerant of multiprocessor

void SwitchToThread(Thread* t);

Thread* GetCurrentThread();
Thread* GetNextThread();

void HandleTaskSwitch();
} // namespace Scheduler

#endif
