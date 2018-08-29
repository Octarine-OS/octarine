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

#include "atkeyboard.h"
#include <portio.h>

#define STATUS_REG 0x64

#define CODE_BACKSPACE  0x0E
#define CODE_ENTER 0x1C
#define CODE_LSHIFT 0x2A
#define CODE_RSHIFT 0x36
#define CODE_EXTENDED 0xE0
#define CODE_SPACE 0x39


static struct {
    int shift:1;
    // int release:1;
    int extended:1;
} flags;

static const char lowerKeyMapping[] = {
    /* 0x02 */ '1',    '2',
    /* 0x04 */ '3',    '4',
    /* 0x06 */ '5',    '6',
    /* 0x08 */ '7',    '8',
    /* 0x0a */ '9',    '0',
    /* 0x0c */ '-',    '=',
    /* 0x0e */ 0,      0,
    /* 0x10 */ 'q',    'w',
    /* 0x12 */ 'e',    'r',
    /* 0x14 */ 't',    'y',
    /* 0x16 */ 'u',    'i',
    /* 0x18 */ 'o',    'p',
    /* 0x1a */ '[',    ']',
    /* 0x1c */ 0,      0,
    /* 0x1e */ 'a',    's',
    /* 0x20 */ 'd',    'f',
    /* 0x22 */ 'g',    'h',
    /* 0x24 */ 'j',    'k',
    /* 0x26 */ 'l',    ';',
    /* 0x28 */ '\'',   '`',
    /* 0x2a */ 0,      '\\',
    /* 0x2c */ 'z',    'x',
    /* 0x2e */ 'c',    'v',
    /* 0x30 */ 'b',    'n',
    /* 0x32 */ 'm',    ',',
    /* 0x34 */ '.',    '/',
};

static const char upperKeyMapping[] = {
    /* 0x02 */ '!',    '@',
    /* 0x04 */ '#',    '$',
    /* 0x06 */ '%',    '^',
    /* 0x08 */ '&',    '*',
    /* 0x0a */ '(',    ')',
    /* 0x0c */ '_',    '+',
    /* 0x0e */ 0,      0,
    /* 0x10 */ 'Q',    'W',
    /* 0x12 */ 'E',    'R',
    /* 0x14 */ 'T',    'Y',
    /* 0x16 */ 'U',    'I',
    /* 0x18 */ 'O',    'P',
    /* 0x1a */ '{',    '}',
    /* 0x1c */ 0,      0,
    /* 0x1e */ 'A',    'S',
    /* 0x20 */ 'D',    'F',
    /* 0x22 */ 'G',    'H',
    /* 0x24 */ 'J',    'K',
    /* 0x26 */ 'L',    ':',
    /* 0x28 */ '"',   '~',
    /* 0x2a */ 0,      '|',
    /* 0x2c */ 'Z',    'X',
    /* 0x2e */ 'C',    'V',
    /* 0x30 */ 'B',    'N',
    /* 0x32 */ 'M',    '<',
    /* 0x34 */ '>',    '?',
};

static void init();
static char getKey();
static char getKeyNB();
static bool hasKey();

static void init() {
    flags.shift = 0;
    // flags.release = 0;
    flags.extended = 0;
}

static char getKey() {
    char key = KEY_NONE;
    while (key == KEY_NONE) {
        key = getKeyNB();
    }
    return key;
}

static bool hasKey() {
    return inb(STATUS_REG) & 0x01;
}

static char getKeyNB() {
    // is byte available to read?
    if (!(inb(STATUS_REG) & 0x01))
        return KEY_NONE;
    uint8_t key = inb(0x60);
    if (flags.extended) {
        // finished extended code
        flags.extended = 0;
        return KEY_NONE;
    }

    if (key & 0x80) {  // a release code
        // flags.release = 1;
        if (key == (CODE_LSHIFT | 0x80) || key == (CODE_RSHIFT | 0x80))
            flags.shift = 0;  // shift key has been released
        return KEY_NONE;
    }

    if (key == CODE_EXTENDED) {  // extended code
        flags.extended = 1;
        return KEY_NONE;
    }

    if (key == CODE_LSHIFT || key == CODE_RSHIFT) {
        flags.shift = 1;
        return KEY_NONE;
    }

    if (key == CODE_SPACE) return ' ';
    if (key == CODE_BACKSPACE) return KEY_BACKSPACE;
    if (key == CODE_ENTER) return KEY_ENTER;

    if (key < 0x02) return KEY_NONE;  // dont care about any lower scancodes
    if (key > 0x35) return KEY_NONE;  // dont care about higher scancodes


    if (flags.shift)
        return upperKeyMapping[key - 0x02];
    else
        return lowerKeyMapping[key - 0x02];
}


Keyboard ATKeyboard = {
    .init = init,
    .hasKey = hasKey,
    .getKey = getKey,
    .getKeyNB = getKeyNB
};
