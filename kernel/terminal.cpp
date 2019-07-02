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

#include <terminal.h>
#include <klib.h>
#include <portio.h>
#include <string.h>

#include "util.hpp"
const uint32_t lineSize = 2 * 80;
static uint16_t * const VRAM = (uint16_t*)0xB8000;

Terminal::Terminal() {
    hpos = 0;
    vpos = 0;
    attr = 7 << 8;
    clearScreen();
    setHWCursor(0,0);
}

void Terminal::clearScreen(){
    for(int i =0; i < 80*25; i++) {
        VRAM[i] = attr;
        ((uint16_t*)buffer)[i] = attr;
    }
}

void Terminal::printUInt(uint32_t val){
    char buff[9];
    shittyHexStr32(val, buff);
    buff[8] = '\0';
    printString(buff);
}

void Terminal::putChar(char c) {
    if(c == '\x0A'){
        newLine();
        return;
    }
    if(c == '\x08') {
        if(hpos == 0) {
            if(vpos != 0) {
                vpos--; hpos = 80;
            } else return; //no chars on screen, nothing to do
        }
        buffer[(offset + vpos)%25] [--hpos] = attr;
        VRAM[(vpos*80)+hpos] = attr;
        return;
    }
    uint16_t chr = attr | c;
    if(hpos >= 80) newLine();
    buffer[(offset + vpos) % 25][hpos] = chr;
    VRAM[(vpos*80)+hpos++] = chr;
}

void Terminal::printChar(char c) {
    putChar(c);
    setHWCursor(hpos, vpos);
}

void Terminal::setAttr(uint8_t _attr){
    attr = _attr << 8;
}

void Terminal::newLine() {
    if(vpos < 24) {
        vpos++;
        hpos = 0;
        return;
    }
    vpos = 24;
    hpos = 0;
    scroll();
}

void Terminal::printString(const char* str) {
    int i = 0;
    char c = str[0];
    while (c) {
        if(c == '\n') newLine();
        else  putChar(c);
        c = str[++i];
    }
    setHWCursor(hpos, vpos);
}

void Terminal::scroll() {
    //clear the top line of buffer since its about to be the new bottom
    memset(buffer[offset], 0, 160);
    offset = (offset + 1) % 25;
    uint32_t startLines = 25 - offset; //number of lines before buffer wraps
    memcpy(VRAM, buffer[offset], 160 * startLines);

    if(startLines == 25) return; //buffer wrap didnt happen
    memcpy(&VRAM[80 * offset], buffer, 160 * (25- startLines));
};

void Terminal::setCursor(uint8_t x, uint8_t y) {
    if(x >= 80) return; //just ignore invalid params
    if(y >= 25) return;
    hpos = x;
    vpos = y;
}

void Terminal::setHWCursor(uint8_t x, uint8_t y) {
    uint16_t position = (y * 80) + x;

    // taken from OSDev Wiki
    // http://wiki.osdev.org/Text_Mode_Cursor
    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(position&0xFF));
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((position>>8)&0xFF));
};

void Terminal::hideCursor() {
    uint8_t reg;
    outb(0x3D4, 0x0A); //cursor start register
    reg = inb(0x3D5);
    reg |= 0x20; //cursor disable bit
    outb(0x3D5, reg);
}

void Terminal::showCursor() {
    /*uint8_t reg;
    outb(0x3D4, 0x0A);
    reg = inb(0x3D5);
    reg &= 0xDF;
    outb(0x3D5, reg);*/
}
