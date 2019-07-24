# Octarine

This repo contains the kernel-space code for the my hobby Octarine OS

## License

This project is licensed under the BSD 2-Clause "Simplified" License  - see the [LICENSE.md](LICENSE.md) file for details

## Building
### Requirements:
- NASM
- CMake >= 3.12
- Clang + lld
- mtools
- QEMU

(A GNU toolchain can be used but requires building a cross compiler)

From the source directory:

    mkdir build
    cd build
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-clang.cmake ..
    make
    qemu-system-i386 -kernel stage2.bin -debugcon stdio

Note:
If qemu gives you an error along the lines of `Error loading uncompressed kernel without PVH ELF Note`, try adding `-machine type=pc-i440fx-3.1` to the command line

## Source Organization

+ arch/ : Architecture specific code
+ contrib/ : Third-party code
+ include/ : Public header files
+ kernel/ : Core kernel sources
+ klib/ : Kernel support library
