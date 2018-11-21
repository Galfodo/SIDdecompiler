#ifndef SASM_UTIL_H_INCLUDED
#define SASM_UTIL_H_INCLUDED

#include "Types.h"
#include <vector>

namespace SASM {

class Util {
public:
  enum AssemblyError {
    OK = 0,
    NO_INPUT,
    SYNTAX_ERROR,
    UNKNOWN_OPCODE,
    BRANCH_OUT_OF_RANGE,
    VALUE_OUT_OF_RANGE,
    ILLEGAL_ADDRESSING_MODE
  };

  static Hue::Util::String  getLastError();
  static AssemblyError      assembleSimpleLine(std::vector<byte>& output, const char* text, int current_pc);
  static Hue::Util::String  formatOpcode(byte* data, word address, bool includeAddress, bool includeHex, bool includeDisasm);
  static Hue::Util::String  formatMem(byte* data, word address, int columns);
  static inline word        calculateBranchTarget(word pc, byte operand) {
                              word target = operand;
                              if (target > 0x80) {
                                target = target - 0x100;
                              }
                              target += pc + 2;
                              return target;
                            }
  static int                calculateBranchOperand(int pc, int target);
};

}

#endif
