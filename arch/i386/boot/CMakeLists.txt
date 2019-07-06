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

set_kernel_flags()

if(OCTARINE_MULTIBOOT)
	add_executable(stage2.dbg multiboot.asm)
	set(STAGE2_LDSCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/multiboot.ld)
else()
    add_executable(stage1.bin stage1.asm)
    set_target_properties(stage1.bin
                        PROPERTIES
                        LINKER_LANGUAGE "ASM_NASM"
                        LINK_FLAGS "-T${CMAKE_CURRENT_SOURCE_DIR}/stage1.ld -melf_i386")
    add_custom_target(stage1 DEPENDS stage1.bin)
	add_executable(stage2.dbg stage2.asm)
    set(STAGE2_LDSCRIPT linker.ld)
    target_include_directories(stage1.bin  PRIVATE ${octarine_SOURCE_DIR}/kernel/)
endif()



#TODO this is a hack
target_include_directories(stage2.dbg PRIVATE ${octarine_SOURCE_DIR}/kernel/)

target_link_libraries(stage2.dbg kernel)

set_target_properties(stage2.dbg
                      PROPERTIES
                      LINKER_LANGUAGE "ASM_NASM"
                      LINK_FLAGS "-T${STAGE2_LDSCRIPT} -melf_i386 -Map stage2.map"
                      LINK_DEPENDS "${STAGE2_LDSCRIPT}")

add_custom_command(
    OUTPUT stage2.bin
    DEPENDS stage2.dbg
    #TODO better search/names for strip command
    COMMAND ${CMAKE_COMMAND} -E copy stage2.dbg stage2.bin
    #COMMAND i686-elf-strip -g stage2.bin
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_custom_target(stage2 ALL DEPENDS stage2.bin)
