
#include "ROM.h"

extern "C" unsigned char basic_data[];
extern "C" unsigned char kernal_data[];
extern "C" unsigned char chargen_data[];

namespace SASM {

ROM::ROM(int page, int page_count, byte* data) : m_Page(page), m_PageCount(page_count), m_Data(data) {
  for (int i = 0; i < C64MachineState::MEM_CONFIGURATIONS; ++i) {
    m_EnabledInConfiguration[i] = false;
  }
}

void ROM::onAttach() {
  for (int i = 0; i < C64MachineState::MEM_CONFIGURATIONS; ++i) {
    if (m_EnabledInConfiguration[i]) {
      machine().m_ReadConfig[i].configurePages(m_Page, m_PageCount, false, m_Data);
    }
  }
}

BasicROM::BasicROM() : ROM(0xa0, 0x20, basic_data) {
  m_EnabledInConfiguration[0x07] = true;
  m_EnabledInConfiguration[0x03] = true;
}

KernalROM::KernalROM() : ROM(0xe0, 0x20, kernal_data) {
  m_EnabledInConfiguration[0x07] = true;
  m_EnabledInConfiguration[0x06] = true;
  m_EnabledInConfiguration[0x03] = true;
  m_EnabledInConfiguration[0x02] = true;
}

ChargenROM::ChargenROM() : ROM(0xd0, 0x10, chargen_data) {
  m_EnabledInConfiguration[0x03] = true;
  m_EnabledInConfiguration[0x02] = true;
  m_EnabledInConfiguration[0x01] = true;
}

}
