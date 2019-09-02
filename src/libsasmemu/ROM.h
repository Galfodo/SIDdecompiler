#ifndef SASM_ROM_H_INCLUDED
#define SASM_ROM_H_INCLUDED

#include "Types.h"
#include "C64MachineState.h"

namespace SASM {

class ROM : public MemoryMappedDevice {
protected:
  byte*         m_Data;
  int           m_Page;
  int           m_PageCount;
  bool          m_EnabledInConfiguration[C64MachineState::MEM_CONFIGURATIONS];
public:
                ROM(int page, int page_count, byte* data);
  virtual void  onAttach() override;
};

class BasicROM : public ROM {
public:
  BasicROM();
};

class KernalROM : public ROM {
public:
  KernalROM();
};

class ChargenROM : public ROM {
public:
  ChargenROM();
};

}

#endif
