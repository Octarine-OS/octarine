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

#include "IRQManager.hpp"
#include "e9dump.hpp"
#include "io/Keyboard.hpp"
#include "portio.h"
#include "util.hpp"
#include <PIC.hpp>

namespace {

class PS2Keyboard : public io::Keyboard {
  public:
	constexpr PS2Keyboard() : flags(0) {}
	~PS2Keyboard() override {}
	int ioctl(uint32_t req, void* data) override;
	void Init();

	// TODO this should maybe be private?
	// although i guess its irrelevant since we are in an anon namespace
	void IRQ();

  private:
	void FlushData();
	enum StateFlags { STATE_KEY_UP = 1 << 0, STATE_E0 = 1 << 1 };
	uint32_t flags;
};

const uint16_t CMD_PORT = 0x64;
const uint16_t DATA_PORT = 0x60;
const uint16_t STATUS_PORT = 0x64;

PS2Keyboard theKeyboard;
keycode_t set_2_keycode_map[]{
    KEY_NONE,     KEY_F9,        KEY_NONE,       KEY_F5,        KEY_F3,
    KEY_F1,       KEY_F2,        KEY_F12,        KEY_NONE,      KEY_F10,
    KEY_F8,       KEY_F6,        KEY_F4,         KEY_TAB,       KEY_GRAVE,
    KEY_NONE,     KEY_NONE,      KEY_LEFTALT,    KEY_LEFTSHIFT, KEY_NONE,
    KEY_LEFTCTRL, KEY_Q,         KEY_1,          KEY_NONE,      KEY_NONE,
    KEY_NONE,     KEY_Z,         KEY_S,          KEY_A,         KEY_W,
    KEY_2,        KEY_NONE,      KEY_NONE,       KEY_C,         KEY_X,
    KEY_D,        KEY_E,         KEY_4,          KEY_3,         KEY_NONE,
    KEY_NONE,     KEY_SPACE,     KEY_V,          KEY_F,         KEY_T,
    KEY_R,        KEY_5,         KEY_NONE,       KEY_NONE,      KEY_N,
    KEY_B,        KEY_H,         KEY_G,          KEY_Y,         KEY_6,
    KEY_NONE,     KEY_NONE,      KEY_NONE,       KEY_M,         KEY_J,
    KEY_U,        KEY_7,         KEY_8,          KEY_NONE,      KEY_NONE,
    KEY_COMMA,    KEY_K,         KEY_I,          KEY_O,         KEY_0,
    KEY_9,        KEY_NONE,      KEY_NONE,       KEY_DOT,       KEY_NONE,
    KEY_L,        KEY_SEMICOLON, KEY_P,          KEY_MINUS,     KEY_NONE,

    KEY_NONE,     KEY_NONE,      KEY_APOSTROPHE, KEY_NONE,      KEY_LEFTBRACE,
    KEY_EQUAL};
/*
0	First PS/2 port interrupt (1 = enabled, 0 = disabled)
1	Second PS/2 port interrupt (1 = enabled, 0 = disabled, only if 2 PS/2 ports
supported) 2	System Flag (1 = system passed POST, 0 = your OS shouldn't be
running) 3	Should be zero 4	First PS/2 port clock (1 = disabled, 0 =
enabled) 5	Second PS/2 port clock (1 = disabled, 0 = enabled, only if 2 PS/2
ports supported) 6	First PS/2 port translation (1 = enabled, 0 = disabled) 7
Must be zero*/
// Config byte stuff
enum  PS2_Config: uint8_t{
	CONFIG_ENABLE_INTR_PORT_1 = 1 << 0,
	CONFIG_ENABLE_INTR_PORT_2 = 1 << 1,
	CONFIG_SYSTEMFLAG = 1 << 2,
	CONFIG_PORT_1_CLOCK = 1 << 4,
	CONFIG_PORT_2_CLOCK = 1 << 5,
	CONFIG_PORT_XLATE = 1 << 6
};

static inline void WriteData(uint8_t data) {
	while (inb(STATUS_PORT) & (1 << 1))
		;
	outb(DATA_PORT, data);
}

static inline void WriteCmd(uint8_t data) {
	// while(true);
	while (inb(STATUS_PORT) & (1 << 1))
		;
	outb(CMD_PORT, data);
}

static uint8_t ReadData() {
	while (~inb(STATUS_PORT) & 1) {
		outb(0xe9, 'S');
	}
	outb(0xe9, '\n');
	return inb(DATA_PORT);
}
// TODO This is a gross hack until we get functors online properly
static void KeyboardIntr() {
	// TODO assert theKeyboard != nullptr
	e9_str("\nKIRQ\n");
	theKeyboard.IRQ();
	PIC::sendEOI(1);
}

void PS2Keyboard::IRQ() {
	uint8_t scancode = inb(DATA_PORT);
	if (scancode == 0xE0) {
		// TODO check if F0 prefix has already been seen. Which would make this
		// an invalid scancode sequence
		this->flags |= STATE_E0;
	} else if (scancode == 0xF0) {
		this->flags |= STATE_KEY_UP;
	} else {
		keycode_t keycode = KEY_NONE;
		if (flags & STATE_E0) {
			// TODO implement
			keycode = KEY_NONE;
		} else {
			// set_2_keycode_map
			if (scancode <
			    (sizeof(set_2_keycode_map) / sizeof(set_2_keycode_map[0]))) {
				keycode = set_2_keycode_map[scancode];
			}
		}
		const bool key_up = (flags & STATE_KEY_UP) != 0;

		flags = 0;
		// abort if we have a bogus keycode
		if (KEY_NONE == keycode)
			return;
		if (key_up) {
			KeyUp(keycode);
		} else {
			KeyDown(keycode);
		}
	}
}

void PS2Keyboard::Init() {
	e9_str("PS2 init\n");
	// Disable both devices
	e9_str("Write cmd\n");
	WriteCmd(0xAD);
	e9_str("Write cmd\n");
	WriteCmd(0xA7);

	// flush the buffer
	FlushData();
	e9_str("Devices Disabled\n");

	// Set configuration
	WriteCmd(0x20);
	uint8_t config = ReadData();
	e9_str("config read\n");
	config &= ~(CONFIG_ENABLE_INTR_PORT_1 | CONFIG_ENABLE_INTR_PORT_2 |
	            CONFIG_PORT_XLATE);
	// config |= CONFIG_ENABLE_INTR_PORT_1;
	// enable clock
	// config |= 1 << 4;

	// config |= CONFIG_ENABLE_INTR_PORT_1;
	// TODO we should be running self test on controller

	WriteCmd(0x60);
	WriteData(config);
	e9_str("Config updated\n");

	// enable the first port
	WriteCmd(0xAE);

	e9_str("sendigRest cmd\n");
	// Send reset command
	FlushData();
	WriteData(0xff);

	uint8_t retcode = ReadData();
	if (retcode == 0xFA) {
		e9_str("RE-read\n");
		retcode = ReadData();
	}
	if (retcode != 0xAA) {
		e9_str("Bad retcode\n");
		e9_dump(retcode);
		outb(0xe9, '\n');
	} else {
		e9_str("Good\n");
	}
	// Enable scan code set 2
	WriteData(0xF0);
	WriteData(2);
	if (ReadData() != 0xFA) {
		e9_str("No ack!");
	}

	// enable scanning
	WriteData(0xF4);
	if (ReadData() != 0xFA) {
		e9_str("No ack!");
	}

	// enable interrupts
	config |= CONFIG_ENABLE_INTR_PORT_1;
	config &= ~CONFIG_PORT_1_CLOCK;
	WriteCmd(0x60);
	WriteData(config);

	// TODO super hack
	IRQManager::SetHandler(1, &KeyboardIntr); // IRQ_1
	FlushData();
}

void PS2Keyboard::FlushData() {
	while (inb(STATUS_PORT) & 2) {
		inb(DATA_PORT);
	}
}

int PS2Keyboard::ioctl(uint32_t req, void* data) { return -1; }
} // namespace

void InitPS2() {
	e9_str("Calling kbd init\n");
	theKeyboard.Init();
}
