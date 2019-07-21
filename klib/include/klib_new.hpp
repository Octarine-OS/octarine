#ifndef KLIB_NEW_HPP
#define KLIB_NEW_HPP

void * operator new(std::size_t __sz);
inline void * operator new(std::size_t sz, void* p) { return p;}
#endif
