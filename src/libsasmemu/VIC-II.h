#ifndef SASM_VICII_H_INCLUDED
#define SASM_VICII_H_INCLUDED

#include "Types.h"
#include "IODevice.h"

namespace SASM {

class ColorRAM : public IODevice {
private:
  ColorRAM();
public:
  byte         m_Data[0x0400];
  virtual void onAttach() override;
  virtual void reset() override;

  friend class VICII;
};

class VICII : public IODevice {
  int         m_VideoMode;
  ColorRAM*   m_ColorRAM;
public:
  enum VideoMode {
    PAL,
    NTSC
  };

  byte         m_Registers[0x40];

               VICII(VideoMode mode = PAL);
  virtual void onAttach() override;
  virtual void reset() override;
  virtual void update(int delta_cycles) override;
  virtual void onReadAccess(int address) override;
  virtual void onWriteAccess(int address) override;
};

}

#endif
