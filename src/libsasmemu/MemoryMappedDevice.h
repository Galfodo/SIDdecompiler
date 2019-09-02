#ifndef SASM_MEMORYMAPPEDDEVICE_H_INCLUDED
#define SASM_MEMORYMAPPEDDEVICE_H_INCLUDED

#include "Types.h"

namespace SASM {

class C64MachineState;

class MemoryMappedDevice {
  C64MachineState* m_MachineState;
protected:
  inline C64MachineState& machine() {
    assert(m_MachineState);
    return *m_MachineState;
  }
  bool          m_IsDirty;
public:

                MemoryMappedDevice();
  virtual       ~MemoryMappedDevice() = 0;
  inline bool   isDirty() const { return m_IsDirty; } // If this is true, update() will be called at the next opportune moment
  inline void   clearDirty() { m_IsDirty = false; } 
  virtual void  onAttach();
  virtual void  onReadAccess(int address);
  virtual void  onWriteAccess(int address);
  virtual void  reset();
  virtual void  update(int delta_cycles);

  friend class C64MachineState;
};

}

#endif
