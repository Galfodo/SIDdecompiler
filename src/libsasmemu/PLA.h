#ifndef SASM_PLA_H_INCLUDED
#define SASM_PLA_H_INCLUDED

#include "Types.h"
#include "C64MachineState.h"

namespace SASM {

class PLA : public MemoryMappedDevice {
public:
  virtual void  onAttach() override;
  virtual void  update(int delta_cycles) override;
  virtual void  onWriteAccess(int address) override;
};

}

#endif
