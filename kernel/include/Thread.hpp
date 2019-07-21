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
#include <klib_new.hpp>
#include <stddef.h>
#include <type_traits>
#include <utility>

template <typename T> class TemplateDebugger {
	template <typename U> class Helper {
		int i;
		char dummy[32];
		static constexpr size_t sz = sizeof(U);
	};
	static_assert(sizeof(Helper<T>) < sizeof(int));
};

class ThreadStack {
	void* _base;
	void* _top;
	template <typename T>
	using SlotType = typename std::remove_reference<T>::type;

  public:
	ThreadStack(ThreadStack&) = delete;
	ThreadStack(ThreadStack&& other);

	template <typename T> SlotType<T>* push(T&& value) {
		using realT = SlotType<T>;
		static_assert(std::is_trivially_destructible<realT>::value,
		              "Pushed value must be trivially destructable");
		// static_assert(std::is_trivially_copy_constructible<realT>::value);
		// using T = int;
		// TODO: should we be doing some checks on T?
		size_t size = sizeof(realT);

		_top = reinterpret_cast<void*>(
		    (reinterpret_cast<uintptr_t>(_top) - size) & ~4);
		new (_top) realT(std::forward<realT>(value));
		return reinterpret_cast<realT*>(_top);
	}

	void* top() { return _top; }

	static ThreadStack make(size_t size);
	~ThreadStack();

  private:
	constexpr ThreadStack(void* base, void* top) : _base(base), _top(top) {}
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

namespace impl {
Thread* _InitThread(ThreadStack stack, void (*entry)(void* arg), void* arg);
}

template <typename T> Thread* InitThread(T&& runnable) {
	static_assert(std::is_invocable<T>::value,
	              "argument must be an invocable object");
	// TODO parameterize stack size
	using BaseType = std::remove_reference_t<T>;
	using FuncType = std::remove_pointer_t<std::remove_reference_t<T>>;
	// TemplateDebugger<typename std::decay<T>::type> x;
	// static_assert(std::is_function_v<BaseType>);
	ThreadStack stack = ThreadStack::make(2048);
	if constexpr (std::is_function_v<FuncType>) {
		FuncType* funcPtr = runnable;
		return impl::_InitThread(
		    std::move(stack),
		    [](void* arg) { (reinterpret_cast<FuncType*>(arg))(); },
		    (void*)funcPtr);
	} else {
		BaseType* functor = stack.push(std::forward<T>(runnable));
		return impl::_InitThread(
		    std::move(stack),
		    [](void* arg) { (*reinterpret_cast<BaseType*>(arg))(); }, functor);
	}
}
// TODO this is all horibly intolerant of multiprocessor

void SwitchToThread(Thread* t);

Thread* GetCurrentThread();
Thread* GetNextThread();

void HandleTaskSwitch();
} // namespace Scheduler

#endif
