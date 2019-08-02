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

find_package(Patch REQUIRED)

set(tarball "${CONTRIB_DIST_DIR}/libcxx-8.0.1.src.tar.xz")
set(tmp_dir "${CMAKE_CURRENT_BINARY_DIR}/tmp")
set(extract_stamp "${tmp_dir}/extract.stamp")
set(patch_stamp "${tmp_dir}/patch.stamp")
set(extract_source_dir "${tmp_dir}/libcxx-8.0.1.src")
set(source_dir "${CMAKE_CURRENT_BINARY_DIR}/libcxx-8.0.1.src")

# Files which we want to extract
set(file_list
	"libcxx-8.0.1.src/include"
	"libcxx-8.0.1.src/CREDITS.TXT"
	"libcxx-8.0.1.src/LICENSE.TXT"
)
set(patches 
	"${CMAKE_CURRENT_SOURCE_DIR}/site_config.patch"
)

configure_file(apply_patches.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/apply_patches.cmake" @ONLY)

if(NOT EXISTS "${tmp_dir}")
	file(MAKE_DIRECTORY "${tmp_dir}")
endif()

# We can download and extract libcxx source at configure time (since dependencies are the same as for re-running cmake)
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

if("${tarball}" IS_NEWER_THAN "${extract_stamp}")
	message(STATUS "Extracting libcxx")
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E tar xJf  "${tarball}" ${file_list}
		WORKING_DIRECTORY "${tmp_dir}"
		RESULT_VARIABLE extract_rc
	)
	if(NOT "${extract_rc}" EQUAL 0)
		file(REMOVE_RECURSE "${extract_source_dir}")
		message(FATAL_ERROR "Failed to extract libcxx (${extract_rc})")
	endif()
	file(TOUCH "${extract_stamp}")
endif()



# build up a list of patch commands
set(patch_commands )
foreach(patch IN LISTS patches)
	list(APPEND patch_commands COMMAND  "${Patch_EXECUTABLE}" "-p1" < "${patch}" )
endforeach()

add_custom_command(
	OUTPUT "${patch_stamp}"
	DEPENDS ${patches} "${CMAKE_CURRENT_BINARY_DIR}/apply_patches.cmake"
	COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/apply_patches.cmake"
	COMMENT "Applying patches to libcxx"
)

add_custom_command(
	OUTPUT "${source_dir}/include/__config_site.hpp"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/__config_site.hpp" "${patch_stamp}"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/__config_site.hpp" "${source_dir}/include/__config_site.hpp"
)

add_custom_target(libcxx_deps
	DEPENDS "${source_dir}/include/__config_site.hpp"
)

add_library(libcxx INTERFACE)
add_dependencies(libcxx libcxx_deps)
target_include_directories(libcxx INTERFACE "${source_dir}/include")
