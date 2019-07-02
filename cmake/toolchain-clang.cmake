#set(CMAKE_SYSTEM_NAME Generic)
#set(CMAKE_SYSTEM_VERSION 1)
#set(CMAKE_SYSTEM_PROCESSOR i686)

#set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)



#set(triple i686-pc-none-elf)

set(CMAKE_C_COMPILER clang)
#set(CMAKE_C_COMPILER_TARGET ${triple})


set(CMAKE_CXX_COMPILER clang++)
#set(CMAKE_CXX_COMPILER_TARGET ${triple})


#set(CMAKE_C_FLAGS " --target=i686-elf -march=i686 -Wall -fno-stack-protector -ffreestanding -m32 -finline-functions -fno-builtin -nostdlib" CACHE STRING "")
#set(CMAKE_CXX_FLAGS "--target=i686-elf -march=i686 -Wall -Wextra -nostdlib -fno-exceptions -fno-rtti -fno-stack-protector -ffreestanding -m32" CACHE STRING "")


set(CMAKE_C_IMPLICIT_LINK_LIBRARIES)
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)

