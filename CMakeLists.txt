cmake_minimum_required(VERSION 2.6)
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


include(arch/${OCTARINE_TARGET_ARCH}/CMakeLists.txt)

add_subdirectory(contrib)

add_subdirectory(src/klib)
add_subdirectory(src/kernel)


if(OCTARINE_MULTIBOOT)
	set(STAGE2_LDSCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/multiboot.ld" )
else()
	set(STAGE2_LDSCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld" )
endif()

add_executable(stage1.bin ${STAGE1_SRCS})
set_target_properties(stage1.bin
                      PROPERTIES
                      LINKER_LANGUAGE "ASM_NASM"
                      LINK_FLAGS "-T${CMAKE_CURRENT_SOURCE_DIR}/stage1.ld -melf_i386")
add_custom_target(stage1 DEPENDS stage1.bin)
target_include_directories(stage1.bin  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/)

add_executable(stage2.dbg ${BOOT_SRCS})

#TODO this is a hack
target_include_directories(stage2.dbg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/)

target_link_libraries(stage2.dbg kernel)

set_target_properties(stage2.dbg
                      PROPERTIES
                      LINKER_LANGUAGE "ASM_NASM"
                      LINK_FLAGS "-T${STAGE2_LDSCRIPT} -melf_i386 -Map stage2.map")

add_custom_command(
    OUTPUT stage2.bin
    DEPENDS stage2.dbg
    #TODO better search/names for strip command
    COMMAND ${CMAKE_COMMAND} -E copy stage2.dbg stage2.bin
    COMMAND i686-elf-strip -g stage2.bin
)
add_custom_target(stage2 ALL DEPENDS stage2.bin)
