#ifndef SASM_SID_H_INCLUDED
#define SASM_SID_H_INCLUDED

#include "Types.h"
#include "IODevice.h"

namespace SASM {

class SID : public IODevice {
  int         m_SIDmodel;
public:
  enum Model {
    MOS6581,
    MOS8580
  };

  byte         m_Registers[0x20];

               SID(Model mode = MOS6581);
  virtual void onAttach() override;
  virtual void reset() override;
  virtual void update(int delta_cycles) override;
  virtual void onReadAccess(int address) override;
  virtual void onWriteAccess(int address) override;
};

}

#endif
