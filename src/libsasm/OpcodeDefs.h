#ifndef SASM_OPCODEDEFS_H_INCLUDED
#define SASM_OPCODEDEFS_H_INCLUDED

#include "AddrMode.h"
#include "Op.h"
#include "Token.h"
#include "Types.h"

#include <vector>

namespace SASM {

struct OpcodeDef {
  OpcodeDef(const char* name, AddrMode addrmode, Op op, int cycles, int page_crossing_penalty = 0);

  const char* m_Name;
  AddrMode    m_AddrMode;
  Op          m_Op;
  int         m_Cycles;
  int         m_PageCrossingPenalty;
  int         m_Opcode;       
};

class OpcodeDefs {
public:
  static int                            getOperandSize(AddrMode addrmode);
  static int                            getOperandSize(OpcodeDef* def);
  static int                            getOperandSize(byte opcode);
  static std::vector<OpcodeDef*> const* checkIsInstruction(Token const& token);
  static std::vector<OpcodeDef*> const* checkIsInstruction(Hue::Util::String const& str);
  static bool                           isBranchInstruction(Op op);
  static OpcodeDef*                     getOpcodeDefForAddressingMode(std::vector<OpcodeDef*> const* opcodedefs, AddrMode addrmode);
  static inline OpcodeDef*              getOpcodeDef(byte opcode) {
                                          return &s_Opcodes[opcode];
                                        }

  static OpcodeDef                      s_Opcodes[256];
};

}

#endif