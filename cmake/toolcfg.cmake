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

include(${CMAKE_CURRENT_LIST_DIR}/libcxx.cmake)

macro(set_kernel_flags)
    set(common_flags
        --target=i686-elf
        -march=i686
        -m32
        -Wall
        -fno-stack-protector
        -ffreestanding
        -finline-functions
        -fno-builtin
        -nostdlib
        -nostdlibinc
    )
    list(JOIN common_flags " " common_flags)


    set(CMAKE_C_FLAGS "${common_flags} ${CMAKE_C_FLAGS}")

    set(_cxx_stdlib_include "")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        get_libcpp_include(_libcpp_include)
        foreach(pth IN LISTS _libcpp_include)
            set(_cxx_stdlib_include "${_cxx_stdlib_include} -cxx-isystem ${pth}")
        endforeach()
    endif()
    set(CMAKE_CXX_FLAGS "${common_flags} -D__ELF__ -D_LIBCPP_HAS_NO_THREADS -fno-exceptions -fno-rtti  ${_cxx_stdlib_include} ${CMAKE_CXX_FLAGS}" )
endmacro()

if(CMAKE_GENERATOR STREQUAL "Ninja")
    set(CMAKE_C_FLAGS "-fcolor-diagnostics ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "-fcolor-diagnostics ${CMAKE_CXX_FLAGS}")
endif()
