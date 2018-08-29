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

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf)
#TODO should remove "i686-elf-ld" literal
set(CMAKE_ASM_NASM_LINK_EXECUTABLE "i686-elf-ld <FLAGS> <CMAKE_ASM_NASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
enable_language(ASM_NASM)

function(nasm_inc_string _output)
message( "new inc string ")
    set(nasm_incOptions "-i${CMAKE_CURRENT_SOURCE_DIR}")
    get_property(nasm_inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)

    FOREACH(nasm_incdir ${nasm_inc_dirs})
        #string(CONCAT incOptions "${incOptions}" "-i ${incdir}/ ")
        #set(incOptions "${incOptions} -i ${incdir}/ ")
        list(APPEND nasm_incOptions "-i ${nasm_incdir}/")
		message( "new output ")
    ENDFOREACH(nasm_incdir)
    list(APPEND nasm_incOptions "-i ${CMAKE_CURRENT_SOURCE_DIR}/")
    set(${_output} ${nasm_incOptions} PARENT_SCOPE)
    #unset(nasm_incOptions)
    #unset(nasm_inc_dirs)
endfunction()

function(invoke_nasm source format output)
	message("invoke nasm")
    nasm_inc_string(_iOpts)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${output}
        COMMAND nasm -f ${format} ${_iOpts} ${CMAKE_CURRENT_SOURCE_DIR}/${source} -o ${CMAKE_CURRENT_BINARY_DIR}/${output} -l ${CMAKE_CURRENT_BINARY_DIR}/${output}.lst
        MAIN_DEPENDENCY ${SOURCE}
    )

    SET_SOURCE_FILES_PROPERTIES(
        "${CMAKE_CURRENT_BINARY_DIR}/${outname}"
        PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )
    unset(_iOpts)
endfunction()

macro(nasm_bin source outname)
    invoke_nasm(${source} "bin" ${outname})
endmacro()

macro(nasm_obj source)
    invoke_nasm(${source} "elf" "${source}.o")
endmacro()
