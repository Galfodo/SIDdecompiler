
#include "CIA.h"

namespace SASM {

CIA::Timer::Timer(int id) : m_CIA(nullptr), m_ID(id), m_Register(id == 0 ? 0x04 : 0x06), m_Underflow(false) {
}

void CIA::Timer::init(CIA* cia) {
  m_CIA = cia;
}

int  CIA::Timer::currentValue() { 
  return getWord(m_CIA->m_ReadRegisters + m_Register);
}

int  CIA::Timer::startValue() { 
  return getWord(m_CIA->m_WriteRegisters + m_Register);
}

void CIA::Timer::loadStartValue() { 
  setCurrentValue(getWord(m_CIA->m_WriteRegisters + m_Register));
}

void CIA::Timer::setCurrentValue(int value) { 
  putWord(m_CIA->m_ReadRegisters + m_Register, value);
}

void CIA::Timer::setStartValue(int value) { 
  putWord(m_CIA->m_WriteRegisters + m_Register, value);
}

void CIA::Timer::reset() {
  assert(m_CIA);
  m_RunMode       = CIA::Timer::STOPPED;
  m_Underflow     = false;
  setCurrentValue(0);
  setStartValue(0);
}

bool CIA::Timer::isUnderflow() { 
  bool isUnderflow = m_Underflow;
  m_Underflow = false;
  return isUnderflow;
}

void CIA::Timer::update(int delta_cycles) {
  switch (m_RunMode) {
  case Timer::CONTINUOUS:
  case Timer::ONCE:
    int value = currentValue() - delta_cycles;
    if (value < 0) {
      m_Underflow = true;
      if (m_RunMode == Timer::CONTINUOUS) {
        value = startValue() + value;
      } else {
        m_RunMode = Timer::STOPPED;
        value = 0;
      }
    }
    setCurrentValue(value);
  }
}

CIA::CIA(CIA::ID id) : 
  m_ID(id), 
  m_Page(id == CIA::CIA_1 ? 0xdc : 0xdd), 
  m_ReadAccess(-1), 
  m_WriteAccess(-1), 
  m_TimerA(Timer::TIMER_A), 
  m_TimerB(Timer::TIMER_B) 
{
  m_TimerA.init(this);
  m_TimerB.init(this);
  reset();
}

void CIA::onAttach() {
  configurePages(m_Page, 1, false, m_ReadRegisters, m_WriteRegisters, 0x0f, true);
}

void CIA::reset() {
  memset(m_ReadRegisters, 0, sizeof(m_ReadRegisters));
  memset(m_WriteRegisters, 0, sizeof(m_WriteRegisters));
  m_InterruptCtrl = 0;
  m_TimerA.reset();
  m_TimerB.reset();
}

int CIA::getWord(byte* data) {
  return data[0] | (data[1] << 8);
}

void CIA::putWord(byte* data, int value) {
  data[0] = value & 0xff; value >>= 8;
  data[1] = value & 0xff;
}
 
void CIA::update(int delta_cycles) {
  //if (m_ReadAccess >= 0 || m_WriteAccess >= 0 || m_TimerA.m_RunMode != Timer::STOPPED || m_TimerB.m_RunMode != Timer::STOPPED) {
  //  int debug = 0;
  //}
  m_TimerA.update(delta_cycles);
  m_TimerB.update(delta_cycles);
  switch(m_ReadAccess) {
  case 0x0d:
    m_ReadRegisters[m_ReadAccess] = 0;
    break;
  }
  switch(m_WriteAccess) {
  case 0x0d:
    {
      byte value = m_WriteRegisters[m_WriteAccess];
      if (value & CtrlFlags::SET_CLEAR_VALUE) {
        m_InterruptCtrl |= value & 0x7f;
      } else {
        value ^= 0x7f;
        m_InterruptCtrl &= value;
      }
    }
    break;
  case 0x0e:
  case 0x0f:
    {
      byte value = m_WriteRegisters[m_WriteAccess];
      m_ReadRegisters[m_WriteAccess] = value;
      Timer& timer = m_WriteAccess == 0x0e ? m_TimerA : m_TimerB;
      if (value & TimerACtrlFlags::TIMER_A_LOAD) {
        timer.loadStartValue();
      }
      if (value & TimerACtrlFlags::TIMER_A_ONCE) {
        timer.m_RunMode = Timer::ONCE;
      } else {
        timer.m_RunMode = Timer::CONTINUOUS;
      }
      if ((value & TimerACtrlFlags::TIMER_A_START) == 0) {
        timer.m_RunMode = Timer::STOPPED;
      }
    }
    break;
  }
  m_IsDirty = !(m_TimerA.m_RunMode == CIA::Timer::STOPPED && m_TimerB.m_RunMode == CIA::Timer::STOPPED);
  byte status = 0;
  if (m_TimerA.isUnderflow()) {
    status |= StatusFlags::TIMER_A_UNDERFLOW;
  }
  if (m_TimerB.isUnderflow()) {
    status |= StatusFlags::TIMER_B_UNDERFLOW;
  }
  if (m_InterruptCtrl & status) {
    status |= StatusFlags::INTERRUPT;
  }
  status |= m_ReadRegisters[0x0d];
  m_ReadRegisters[0x0d] = status;
  if (status & StatusFlags::INTERRUPT) {
    if (m_ID == CIA_1) {
      machine().setIRQ();
    } else {
      machine().setNMI();
    }
    m_IsDirty = true;
  }
  m_ReadAccess = -1;
  m_WriteAccess = -1;
}

void CIA::onReadAccess(int address) {
  int reg = address & 0x0f;
  switch(reg) {
  case 0x0d:
    m_ReadAccess = reg;
    m_IsDirty = true;
  }
}

void CIA::onWriteAccess(int address) {
  int reg = address & 0x0f;
  switch(reg) {
  case 0x0d:
  case 0x0e:
  case 0x0f:
    m_WriteAccess = reg;
    m_IsDirty = true;
  }
}

}
