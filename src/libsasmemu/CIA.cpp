
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

CIA::CIA(CIA::ID id) : m_ID(id) {
  reset();
}

void CIA::reset() {
  m_AssertInterrupt = false;
  m_TimerA.reset();
  m_TimerB.reset();
}

void CIA::update(int delta_cycles) {
  m_TimerA.update(delta_cycles);
  m_TimerB.update(delta_cycles);
}

}
