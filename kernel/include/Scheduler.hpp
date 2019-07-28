/*
 * Copyright (c) 2019, Devin Nakamura
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

#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "Thread.hpp"

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
		    stack, [](void* arg) { (reinterpret_cast<FuncType*>(arg))(); },
		    (void*)funcPtr);
	} else {
		BaseType* functor = stack.push(std::forward<T>(runnable));
		return impl::_InitThread(
		    stack, [](void* arg) { (*reinterpret_cast<BaseType*>(arg))(); },
		    functor);
	}
}
void TaskSwitchIRQ(arch::Context* state);
} // namespace Scheduler

#endif
