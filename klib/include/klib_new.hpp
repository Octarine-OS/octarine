#ifndef KLIB_NEW_HPP
#define KLIB_NEW_HPP

#include <stddef.h>

void * operator new(size_t __sz);
inline void * operator new(size_t sz, void* p) { return p;}
#endif
