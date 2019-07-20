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

# download and extract libc++ return include path in out_var
function(get_libcpp_include out_var)
	set(tarball "${CMAKE_BINARY_DIR}/dist/libcxx-8.0.1.src.tar.xz")
	set(source_dir "${CMAKE_BINARY_DIR}/libcxx-8.0.1.src")

	set(file_list
		"libcxx-8.0.1.src/include"
		"libcxx-8.0.1.src/CREDITS.TXT"
		"libcxx-8.0.1.src/LICENSE.TXT"
	)

	if(NOT IS_DIRECTORY "${CMAKE_BINARY_DIR}/dist")
		file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/dist")
	endif()
	if(NOT EXISTS "${tarball}")
		message(STATUS "Downloading libcxx")
		file(DOWNLOAD
			"https://github.com/llvm/llvm-project/releases/download/llvmorg-8.0.1/libcxx-8.0.1.src.tar.xz"
			"${tarball}"
			SHOW_PROGRESS
			STATUS dl_status
		)
		list(GET dl_status 0 dl_rc)
		if(NOT "${dl_rc}"  EQUAL 0)
			file(REMOVE "${tarball}")
			message(FATAL_ERROR "Failed to download libcxx (${dl_status})")
		endif()
	endif()
	if(NOT EXISTS "${source_dir}/extract.stamp")
		message(STATUS "Extracting libcxx")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xJf  "${tarball}" ${file_list}
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
			RESULT_VARIABLE extract_rc
			#ECH
		)
		if(NOT "${extract_rc}" EQUAL 0)
			file(REMOVE_RECURSE "${source_dir}")
			message(FATAL_ERROR "Failed to extract libcxx (${extract_rc})")
		endif()
		file(TOUCH "${source_dir}/extract.stamp")
	endif()
	set(${out_var} "${source_dir}/include" PARENT_SCOPE)
endfunction()
