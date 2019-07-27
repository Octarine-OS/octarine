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

/**
 * /file
 * Kernel panic handling code
 */

#include "terminal.h"
#include "util.hpp"
#include <Octarine.hpp>
// TODO panics should maybe switch to an emergency stack?
static void panicCore(const char* msg) {
	Terminal& term = *globalTerm;
	term.setAttr(0x47);
	term.clearScreen();
	term.setCursor(1, 0);
	term.printString("KERNEL PANIC!:");
	// TODO: we should probably have a hard limit on the length of message
	if (msg) {
		term.setCursor(1, 1);
		term.printString(msg);
	}
}

static void dumpRegister(int line, int col, const char* name, uint32_t val) {
	Terminal& term = *globalTerm;
	char buff[5];
	buff[4] = 0;
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

// TODO this is arch specific
static void dumpContext(const arch::Context& ctx) {
	dumpRegister(10, 0, "EAX", ctx.eax);
	dumpRegister(10, 1, "ECX", ctx.ecx);

	dumpRegister(11, 0, "EDX", ctx.edx);
	dumpRegister(11, 1, "EBX", ctx.ebx);

	dumpRegister(12, 0, "ESP", ctx.esp);
	dumpRegister(12, 1, "EBP", ctx.ebp);

	dumpRegister(13, 0, "ESI", ctx.esi);
	dumpRegister(13, 1, "EDI", ctx.edi);

	dumpRegister(14, 0, "EIP", ctx.eip);
}

void panic(const char* msg) {
	panicCore(msg);
	while (true) {
		// TODO make sure the compiler won't optimize this out
	}
}

void panic(arch::Context& ctx, const char* msg) {
	panicCore(msg);
	dumpContext(ctx);
	while (true) {
		// TODO make sure the compiler won't optimize this out
	}
}
