
#include "CIA.h"

namespace SASM {

CIA::Timer::Timer() {
  reset();
}

void CIA::Timer::reset() {
  m_RunMode       = CIA::Timer::STOPPED;
  m_InitValue     = 0;
  m_Value.m_asInt = 0;
  m_Underflow     = false;
}

void CIA::Timer::update(int delta_cycles) {
  switch (m_RunMode) {
  case Timer::CONTINUOUS:
  case Timer::ONCE:
    m_Value.m_asInt -= delta_cycles;
    if (m_Value.m_asInt < 0) {
      m_Underflow = true;
      if (m_RunMode == Timer::CONTINUOUS) {
        m_Value.m_asInt = m_InitValue + m_Value.m_asInt;
      } else {
        m_RunMode = Timer::STOPPED;
        m_Value.m_asInt = 0;
      }
    }
  }
}

CIA::CIA(CIA::ID id) : m_ID(id), m_Page(id == CIA::CIA_1 ? 0xdc : 0xdd) {
  reset();
}

class CIAWriteEvaluator : public TraceEvaluator {
  virtual int eval(C64MachineState&, int) {
    return TraceEvaluator::CONTINUE;
  }
};

void CIA::onAttach() {
  configurePages(m_Page, 1, false, m_ReadData, m_WriteData, 0x0f, true);
}

void CIA::reset() {
  m_AssertInterrupt = false;
  memset(m_ReadData, 0, sizeof(m_ReadData));
  memset(m_WriteData, 0, sizeof(m_WriteData));
  m_TimerA.reset();
  m_TimerB.reset();
}

void CIA::update(int delta_cycles) {
  m_TimerA.update(delta_cycles);
  m_TimerB.update(delta_cycles);
  m_ReadData[4] = m_TimerA.m_Value.m_asInt & 0xff;
  m_ReadData[5] = (m_TimerA.m_Value.m_asInt >> 8) & 0xff;  
  m_IsDirty = m_TimerA.m_RunMode != CIA::Timer::STOPPED;
}

void CIA::onReadAccess(int address) {
}

void CIA::onWriteAccess(int address) {
}

}
