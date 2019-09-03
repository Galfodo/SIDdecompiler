#ifndef SASM_CIA_H_INCLUDED
#define SASM_CIA_H_INCLUDED

#include "Types.h"
#include "IODevice.h"

namespace SASM {

class CIA : public IODevice {
  int m_Page;
public:
  class Timer {
    CIA* m_CIA;
    bool m_Underflow;
    int  m_ID;
    int  m_Register;

  public:
    enum ID {
      TIMER_A = 0,
      TIMER_B
    };

    enum RunMode {
      STOPPED = 0,
      ONCE,
      CONTINUOUS
    } m_RunMode;

    Timer(int id);
    void init(CIA* cia);
    int  currentValue();
    int  startValue();
    void loadStartValue();
    void setCurrentValue(int value);
    void setStartValue(int value);
    void reset();
    void update(int delta_cycles);
    bool isUnderflow();
  };

  enum {
    PAL_60Hz = 0x4025,
    NTSC_60Hz = 0x4295
  };

  typedef enum {
    CIA_1 = 0,
    CIA_2

  } ID;

  enum StatusFlags {
    TIMER_A_UNDERFLOW = 0x01,
    TIMER_B_UNDERFLOW = 0x02,
    TOD_ALARM         = 0x04,
    BYTE_READY        = 0x08,
    FLAG              = 0x10,
    INTERRUPT         = 0x80
  };

  enum CtrlFlags {
    TIMER_A_ENABLE_INT    = 0x01,
    TIMER_B_ENABLE_INT    = 0x02,
    TOD_ALARM_ENABLE_INT  = 0x04,
    BYTE_READY_ENABLE_INT = 0x08,
    FLAG_ENABLE_INT       = 0x10,
    SET_CLEAR_VALUE       = 0x80 // Flags set to 1 will get the value of this bit. The others are untouched.
  };

  enum TimerACtrlFlags {
    TIMER_A_START = 0x01, // 0 = Stop timer; 1 = Start timer.
    TIMER_A_BIT_1 = 0x02, // 1 = Indicate timer underflow on port B bit #6.
    TIMER_A_BIT_2 = 0x04, // 0 = Upon timer underflow, invert port B bit #6; 1 = upon timer underflow, generate a positive edge on port B bit #6 for 1 system cycle.
    TIMER_A_ONCE  = 0x08, // 0 = Timer restarts upon underflow; 1 = Timer stops upon underflow.
    TIMER_A_LOAD  = 0x10, // 1 = Load start value into timer.
    TIMER_A_BIT_5 = 0x20, // 0 = Timer counts system cycles; 1 = Timer counts positive edges on CNT pin.
    TIMER_A_BIT_6 = 0x40, // Serial shift register direction; 0 = Input, read; 1 = Output, write.
    TIMER_A_BIT_7 = 0x80  // TOD speed; 0 = 60 Hz; 1 = 50 Hz.  
  };

  enum TimerBCtrlFlags {
    TIMER_B_START = 0x01, // 0 = Stop timer; 1 = Start timer.
    TIMER_B_BIT_1 = 0x02, // 1 = Indicate timer underflow on port B bit #7.
    TIMER_B_BIT_2 = 0x04, // 0 = Upon timer underflow, invert port B bit #7; 1 = upon timer underflow, generate a positive edge on port B bit #7 for 1 system cycle.
    TIMER_B_ONCE  = 0x08, // 0 = Timer restarts upon underflow; 1 = Timer stops upon underflow.
    TIMER_B_LOAD  = 0x10, // 1 = Load start value into timer.
    TIMER_B_BIT_5 = 0x20, // Bit 5 & 6: %00 = Timer counts system cycles; %01 = Timer counts positive edges on CNT pin; %10 = Timer counts underflows of timer A; %11 = Timer counts underflows of timer A occurring along with a positive edge on CNT pin.
    TIMER_B_BIT_6 = 0x40, //
    TIMER_B_BIT_7 = 0x80  // 0 = Writing into TOD registers sets TOD; 1 = Writing into TOD registers sets alarm time.  
  };

  ID            m_ID;
  Timer         m_TimerA; 
  Timer         m_TimerB;
  byte          m_InterruptCtrl; 
  byte          m_ReadRegisters[16];
  byte          m_WriteRegisters[16];
  int           m_ReadAccess;
  int           m_WriteAccess;

                CIA(ID id);
  static int    getWord(byte* data);
  static void   putWord(byte* data, int word);
  virtual void  onAttach() override;
  virtual void  reset() override;
  virtual void  update(int delta_cycles) override;
  virtual void  onReadAccess(int address) override;
  virtual void  onWriteAccess(int address) override;
};

}

#endif 
