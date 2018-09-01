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

#include "kernel/driveio.h"
#include "kernel/registers.h"
#include "kernel/bioscall.h"

#include <portio.h>

Drive::Drive(uint8_t drivenum) {
    num = drivenum;
}

int Drive::chsRead(CHS_Address addr, uint8_t numSectors, void *buffer) {
    // check if addr is outside adressing range
    if ((uint32_t)buffer & ~0xFFFFF) return -1;
    Registers reg;
    uint32_t sz = sizeof(reg);
    outb(0xe9, sz & 0xFF);
    reg.ah = 0x02;
    reg.al = numSectors;
    reg.ch = addr.cylinder & 0xFF;
    // cl contains low 6bits of sector and top 2 bits of cyl
    reg.cl = addr.sector | ((addr.cylinder >> 2) & 0xC0);
    reg.dh = addr.head;
    reg.dl = num;
    reg.es = ((uint32_t) buffer) >> 4;
    reg.bx = ((uint32_t) buffer) & 0xF;
    callBios(0x13, &reg);
    if (reg.flags & 0x1) return -1;
    return 0;
}

int Drive::getLastStatus() {
    Registers reg;
    reg.ah = 0x01;
    reg.al = this->num;
    callBios(0x13, &reg);
    return reg.ah;  // status is returned in ah
}
