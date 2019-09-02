
#include "SID.h"

namespace SASM {


SID::SID(Model model) : m_SIDmodel(model) {
}

void SID::onAttach() {
  configurePages(0xd4, 4, true, m_Registers, m_Registers, 0x1f, true);
}

void SID::reset() {
  memset(m_Registers, 0, sizeof(m_Registers));
}

void SID::update(int delta_cycles) {
}

void SID::onReadAccess(int address) {
}

void SID::onWriteAccess(int address) {
}

}
