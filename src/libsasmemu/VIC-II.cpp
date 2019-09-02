
#include "VIC-II.h"

namespace SASM {

ColorRAM::ColorRAM() {
  reset();
}

void ColorRAM::onAttach() {
  configurePages(0xd8, 4, false, m_Data, m_Data, 0xff, false);
}

void ColorRAM::reset() {
  memset(m_Data, 0xff, sizeof(m_Data));
}

VICII::VICII(VideoMode mode) : m_VideoMode(mode), m_ColorRAM(new ColorRAM()) {
}

void VICII::onAttach() {
  configurePages(0xd0, 4, true, m_Registers, m_Registers, 0x3f, true);
  machine().attach(m_ColorRAM);
}

void VICII::reset() {
  memset(m_Registers, 0, sizeof(m_Registers));
}

void VICII::update(int delta_cycles) {
}

void VICII::onReadAccess(int address) {
}

void VICII::onWriteAccess(int address) {
}

}
