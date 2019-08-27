#ifndef SASM_CIA_H_INCLUDED
#define SASM_CIA_H_INCLUDED

#include "Types.h"
#include "C64MachineState.h"

namespace SASM {
  
class CIA : public MemoryMappedDevice {
public:
  struct Timer {
    union {
      int  m_asInt;
      byte m_asBytes[4];
    } m_Value;
    int m_InitValue; 
    enum RunMode {
      STOPPED = 0,
      ONCE,
      CONTINUOUS
    } m_RunMode;

    bool m_Underflow;

    Timer();
    void reset();
    void update(int delta_cycles);
  };

  enum {
    PAL_60Hz = 0x4025,
    NTSC_60Hz = 0x4295
  };

  typedef enum {
    CIA_1,
    CIA_2

  } ID;

  enum StatusFlags : byte {
    TIMER_A_UNDERFLOW = 0x01,
    TIMER_B_UNDERFLOW = 0x02,
    TOD_ALARM         = 0x04,
    BYTE_READY        = 0x08,
    FLAG              = 0x10,
    INTERRUPT         = 0x80
  };

  enum CtrlFlags : byte {
    TIMER_A_ENABLE_INT    = 0x01,
    TIMER_B_ENABLE_INT    = 0x02,
    TOD_ALARM_ENABLE_INT  = 0x04,
    BYTE_READY_ENABLE_INT = 0x08,
    FLAG_ENABLE_INT       = 0x10,
    SET_CLEAR_VALUE       = 0x80 // Flags set to 1 will get the value of this bit. The others are untouched.
  };

  enum class TimerACtrlFlags {
    START = 0x01, // 0 = Stop timer; 1 = Start timer.
    BIT_1 = 0x02, // 1 = Indicate timer underflow on port B bit #6.
    BIT_2 = 0x04, // 0 = Upon timer underflow, invert port B bit #6; 1 = upon timer underflow, generate a positive edge on port B bit #6 for 1 system cycle.
    BIT_3 = 0x08, // 0 = Timer restarts upon underflow; 1 = Timer stops upon underflow.
    BIT_4 = 0x10, // 1 = Load start value into timer.
    BIT_5 = 0x20, // 0 = Timer counts system cycles; 1 = Timer counts positive edges on CNT pin.
    BIT_6 = 0x40, // Serial shift register direction; 0 = Input, read; 1 = Output, write.
    BIT_7 = 0x80  // TOD speed; 0 = 60 Hz; 1 = 50 Hz.  
  };

  enum class TimerBCtrlFlags : byte {
    BIT_0 = 0x01, // 0 = Stop timer; 1 = Start timer.
    BIT_1 = 0x02, // 1 = Indicate timer underflow on port B bit #7.
    BIT_2 = 0x04, // 0 = Upon timer underflow, invert port B bit #7; 1 = upon timer underflow, generate a positive edge on port B bit #7 for 1 system cycle.
    BIT_3 = 0x08, // 0 = Timer restarts upon underflow; 1 = Timer stops upon underflow.
    BIT_4 = 0x10, // 1 = Load start value into timer.
    BIT_5 = 0x20, // Bit 5 & 6: %00 = Timer counts system cycles; %01 = Timer counts positive edges on CNT pin; %10 = Timer counts underflows of timer A; %11 = Timer counts underflows of timer A occurring along with a positive edge on CNT pin.
    BIT_6 = 0x40, //
    BIT_7 = 0x80  // 0 = Writing into TOD registers sets TOD; 1 = Writing into TOD registers sets alarm time.  
  };

  ID          m_ID;
  Timer       m_TimerA; 
  Timer       m_TimerB;
  byte        m_InterruptCtrl; 
  byte        m_Status;
  byte        m_TimerACtrl;
  byte        m_TimerBCtrl;
  bool        m_AssertInterrupt;

  CIA(ID id);
  virtual void onAttach(C64MachineState& machine) override;
  virtual void reset() override;
  virtual void update(int delta_cycles) override;
};

}

#endif 
