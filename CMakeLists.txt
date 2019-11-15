################################################################################
# Copyright (c) 2019, Devin Nakamura
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

cmake_minimum_required(VERSION 3.12)
include(cmake/asm.cmake)

project(octarine)

enable_language(ASM_NASM)

include(cmake/config.cmake)
include(cmake/toolcfg.cmake)

set(CMAKE_ASM_NASM_LINKER_PREFERENCE_PROPAGATES 0)
set(CMAKE_CXX_LINKER_PREFERENCE_PROPAGATES 0)
set(CMAKE_C_LINKER_PREFERENCE_PROPAGATES 0) 
set(CMAKE_C_IMPLICIT_LINK_LIBRARIES)
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(tools)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Ui386")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Ui386")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
include_directories (${octarine_SOURCE_DIR}/include/)

# set up some inf


# Contrib needs to be first so that other targets can refer to libcxx
add_subdirectory(contrib)

add_subdirectory(arch)

add_subdirectory(klib)
add_subdirectory(kernel)

if(OCTARINE_BUILD_DOCS)
    add_subdirectory(docs)
endif()
