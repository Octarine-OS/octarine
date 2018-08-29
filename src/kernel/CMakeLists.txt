################################################################################
# Copyright (c) 2018, Devin Nakamura
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# SPDX-License-Identifier: BSD-2-Clause
################################################################################

#add_library(kernel STATIC EXCLUDE_FROM_ALL main.cpp atkeyboard.cpp driveio.cpp terminal.cpp shell.cpp mm.cpp bioscall.asm cpumode.asm)

#hacks for multiboot
# TODO re-add shell
set(COMMON_SRC  bioscall.asm  tlsf.c util.cpp scheduler.cpp cpumode.asm terminal.cpp  mm.cpp atkeyboard.cpp InterruptManager.cpp InterruptHandler_asm.asm pic.cpp mario.asm)

add_library(kernel STATIC EXCLUDE_FROM_ALL
	bioscall.asm
	util.cpp
	scheduler.cpp
	cpumode.asm
	terminal.cpp
	mm.cpp
	atkeyboard.cpp
	InterruptManager.cpp
	InterruptHandler_asm.asm
	Keyboard.cpp
	PS2Keyboard.cpp
	cxx.cpp
)
if(OCTARINE_MULTIBOOT)
	target_sources(kernel PRIVATE MBootMain.cpp)

else()
	target_sources(kernel PRIVATE main.cpp)
endif()

target_include_directories(kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(kernel arch_kernel klib tlsf)