#ifndef SASM_ADDRMODE_H_INCLUDED
#define SASM_ADDRMODE_H_INCLUDED

namespace SASM {

enum class AddrMode {
  IMPL,   //
  XIND,   // (zp,x)
  ZP,     // zp
  IMM,    // #value
  ABS,    // addr
  REL,    // offset
  INDY,   // (zp),y
  ZPX,    // zp,x
  ABSY,   // addr,y
  ABSX,   // addr,x
  IND,    // (addr)
  ZPY     // zp,y
};

}

#endif
