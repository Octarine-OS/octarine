cmake_minimum_required(VERSION 3.12)
include(cmake/asm.cmake)

project(octarine)

enable_language(ASM_NASM)

include(cmake/config.cmake)


set(CMAKE_ASM_NASM_LINKER_PREFERENCE_PROPAGATES 0)
set(CMAKE_CXX_LINKER_PREFERENCE_PROPAGATES 0)
set(CMAKE_C_LINKER_PREFERENCE_PROPAGATES 0) 
set(CMAKE_C_IMPLICIT_LINK_LIBRARIES)
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

include_directories (${octarine_SOURCE_DIR}/include/)

# set up some inf


add_subdirectory(arch)

add_subdirectory(contrib)

add_subdirectory(klib)
add_subdirectory(kernel)
