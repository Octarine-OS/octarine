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

#ifndef REGISTERS_H
#define REGISTERS_H

#include <sys/types.h>

//WARNING - editing this requires changing include/core/kernel/registers.mac
typedef struct {
    union {
        uint32_t eax;
        uint16_t ax;
        struct {
            uint8_t al;
            uint8_t ah;
        };
    };

    union {
        uint32_t ebx;
        uint16_t bx;
        struct {
            uint8_t bl;
            uint8_t bh;
        };
    };

    union {
        uint32_t ecx;
        uint16_t cx;
        struct {
            uint8_t cl;
            uint8_t ch;
        };
    };

    union {
        uint32_t edx;
        uint16_t dx;
        struct {
            uint8_t dl;
            uint8_t dh;
        };
    };

    uint16_t si, di;
    uint16_t ds, es, fs, gs;
    uint16_t flags;
}  __attribute__((packed)) Registers;
#endif
