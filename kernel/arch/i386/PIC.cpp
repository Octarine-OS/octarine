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

#include <PIC.hpp>
#include <portio.h>

// namespace {
const uint16_t PIC1_COMMAND = 0x20;
const uint16_t PIC1_DATA = 0x21;

const uint16_t PIC2_COMMAND = 0xA0;
const uint16_t PIC2_DATA = 0xA1;

const uint8_t PIC_EOI = 0x20;

const uint8_t ICW1_ICW4 = 0x01;
const uint8_t ICW1_LEVEL = 0x08;
// const uint8_t ICW1_INIT = 0x10;
const uint8_t ICW1_INIT = 0x10;
const uint8_t ICW4_8086 = 0x01;

//}

void PIC::init() { PIC::remap(0x20, 0x28); }

void PIC::sendEOI(uint8_t irq) {
	// assert(irq < 16)
	if (irq >= 8) {
		outb(PIC2_COMMAND, PIC_EOI);
	} else {
		outb(PIC1_COMMAND, PIC_EOI);
	}
}

//#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
//#define ICW1_INIT	0x10		/* Initialization - required! */

//#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
//#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
//#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
//#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
//#define ICW4_SFNM	0x10		/* Special fully nested (not) */

/*
arguments:
    offset1 - vector offset for master PIC
        vectors on the master become offset1..offset1+7
    offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC::remap(uint8_t offset1, uint8_t offset2) {
	uint8_t a1, a2;

	a1 = inb(PIC1_DATA); // save masks
	a2 = inb(PIC2_DATA);

	outb(PIC1_COMMAND,
	     ICW1_INIT +
	         ICW1_ICW4); // starts the initialization sequence (in cascade mode)
	// io_wait();
	outb(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
	// io_wait();
	outb(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
	// io_wait();
	outb(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset
	// io_wait();
	outb(PIC1_DATA, 4); // ICW3: tell Master PIC that there is a slave PIC at
	                    // IRQ2 (0000 0100)
	// io_wait();
	outb(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
	// io_wait();

	outb(PIC1_DATA, ICW4_8086);
	// io_wait();
	outb(PIC2_DATA, ICW4_8086);
	// io_wait();

	outb(PIC1_DATA, a1); // restore saved masks.
	outb(PIC2_DATA, a2);
}
