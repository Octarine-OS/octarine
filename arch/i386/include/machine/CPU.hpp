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

#ifndef I386_MACHINE_CPU
#define I386_MACHINE_CPU

#include <Octarine.hpp>

namespace i386 {

inline bool interruptsEnabled() {
	uint32_t rflags;
	asm volatile("pushfd\n"
	             "\rpop %0"
	             : "=r"(rflags));
	return (rflags & 0x200) != 0;
}

namespace impl {
/// \todo this needs to be changed when we have multi cpu support
extern int cliLevel;
inline void enableInterrupts() { asm volatile("sti"); }
inline void disableInterrupts() { asm volatile("cli"); }
} // namespace impl

inline void enterCriticalSection() {
	impl::disableInterrupts();
	KASSERT(impl::cliLevel >= 0);
	++impl::cliLevel;
}

inline void exitCriticalSection() {
	KASSERT(impl::cliLevel > 0);
	--impl::cliLevel;
	if (impl::cliLevel == 0) {
		impl::enableInterrupts();
	}
}
} // namespace i386

namespace arch {
using i386::enterCriticalSection;
using i386::exitCriticalSection;
using i386::interruptsEnabled;
} // namespace arch
#endif
