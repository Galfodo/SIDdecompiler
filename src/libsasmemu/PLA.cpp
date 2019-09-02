
#include "PLA.h"

namespace SASM {

void  PLA::onAttach() {
  for (int i = 0; i < C64MachineState::MEM_CONFIGURATIONS; ++i) {
    machine().m_WriteConfig[i].configurePage(0x00, machine().m_Mem, this);
  }
}

void PLA::onWriteAccess(int address) {
  if (address == 0x0001) {
    m_IsDirty = true;
  }
}

void PLA::update(int delta_cycles) {
  int memconfig = machine().m_Mem[0x0001] & 0x07;
  machine().m_CurrentReadConfig   = &machine().m_ReadConfig[memconfig];
  machine().m_CurrentWriteConfig  = &machine().m_WriteConfig[memconfig];
}

}
