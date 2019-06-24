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
    cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_LINKER=ld.lld ..
    make
    qemu -kernel arch/i386/boot/stage2.bin

## Source Organization

+ arch/ : Architecture specific code
+ contrib/ : Third-party code
+ include/ : Public header files
+ kernel/ : Core kernel sources
+ klib/ : Kernel support library
