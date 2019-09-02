#ifndef SASM_FULLEMU_H_INCLUDED
#define SASM_FULLEMU_H_INCLUDED

#include "C64MachineState.h"

namespace SASM {

class FullEmu : public C64MachineStateT<EmuTraits<true , DebuggerState::TRAP_ALL >, true> 
{
public:
  FullEmu();
};

}

#endif
