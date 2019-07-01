#ifndef I386_MACHINE_CONTEXT
#define I386_MACHINE_CONTEXT

#include <stdint.h>


//note this matches layout of "PUSHAD" func
//TODO: save segment register state when that is an issue
namespace i386 {
struct Context {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  
  uint32_t intNum;
  uint32_t errCode;
  
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
};
}

namespace arch{
using Context = i386::Context;
}


#endif
