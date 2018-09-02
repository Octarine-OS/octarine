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

//Main file for Multiboot loader


#include "atkeyboard.h"
#include "mm.h"
#include "terminal.h"
#include "multiboot.h"
#include "IRQManager.hpp"
#include "pic.hpp"
#include "portio.h"

#include "Thread.hpp"

//TODO this is a hack
#include "../kernel/IDT.hpp"

extern char start_load, end_load, ebss;
extern "C" void _multibootEntry();

extern char _low_mapping, _begin_trampoline, _end_trampoline;

void initTrampoline(){
	char *src = &_begin_trampoline;
	char *dst = &_low_mapping;
	while (src < &_end_trampoline) {
		*dst++ = *src++;
	}
}
//uint8_t port61_val;
uint8_t port61_silence;
uint8_t port61_on;


static inline void setTimer(uint16_t val){
  outb(0x40, val & 0xff);
  outb(0x40, (val >> 8) & 0xff);
}
void shittyHexStr(uint16_t val, char *buff);



void DummyFunc();

const int SPIN_COUNT = 2000;
void Thread1Test(){
	while(true){
		for(int i =0; i < SPIN_COUNT; ++i){
			DummyFunc();
		}
		outb(0xe9, '#');
	}
}

void MaskIntr(){
	outb(0x21, 0xFD);
}
void Thread2Test(){
	while(true){
		for(int i =0; i < SPIN_COUNT; ++i){
			DummyFunc();
		}
		outb(0xe9, '$');
	}
}
//SUPER HACK
void InitPS2();

void GarbageHandler(){

}

Terminal *globalTerm = 0;
extern "C" void kmain(uint32_t magic, multiboot_info *bootInfo) {
	Terminal term;
	globalTerm = &term;
	i386::IDT::Dummy(); //TODO a hack for linking crap
	if(magic != MULTIBOOT_BOOTLOADER_MAGIC){
		term.printString("Bootloader failure!\n");
		return;
	}
	initTrampoline();
	initMM();
    //
    /*
    ATKeyboard.init();
    term.showCursor();
    term.printString("OCTARINE V1.0\n");
    term.printString("Boot drive: ");
    char buff[32];
    ucvt(bootInfo->boot_device, buff, 10, 3, 0);
    term.printString(buff);
    term.newLine();
    runShell(term);*/
	term.printString("MULTIBOOT Proof of Concept!\n");
	//kill time 
	term.printString("initalizing interrupt manager.\n");

	IRQManager::Initialize();
	term.printString("Initializing PIC\n");
	PIC::init();


	InitPS2();

	MaskIntr();
	IRQManager::SetHandler(0, GarbageHandler); //IRQ_0 eg 0x20
	asm("sti");
	if(false){ // stuff for task switching test code
		asm("cli");
		term.printString("Beginning thread test\n");
		//Scheduler::Init();
		Scheduler::InitThread(&Thread1Test)->id = 1;
		Scheduler::InitThread(&Thread2Test)->id = 2;

		//IRQManager::SetHandler(0, TaskSwitchIRQ); //IRQ_0


		//Enable the timer .... i think
		outb(0x43, 0x34);

				//set frequency to approx 20Hz
		setTimer(59659);
		asm("sti");

		//Sit in an idle loop
		// Note due to limitations in the scheduler, once the first task switch hits, we never come back to this thread
		while(true){
			asm("nop");
		}
	}
	while(true);

}

__attribute__((section(".header")))
constexpr multiboot_header _mbootHeader  = {
	.magic = MULTIBOOT_HEADER_MAGIC,
	.flags = MULTIBOOT_AOUT_KLUDGE,
	.checksum =(uint32_t) (0-(MULTIBOOT_HEADER_MAGIC+MULTIBOOT_AOUT_KLUDGE)),
	.header_addr = (uint32_t) &_mbootHeader,
	.load_addr = (uint32_t) &_mbootHeader,
	//.load_addr = (uint32_t) &start_load,
	.load_end_addr = (uint32_t) &end_load,
	.bss_end_addr = (uint32_t) &ebss,
	.entry_addr = (uint32_t)&_multibootEntry
};
