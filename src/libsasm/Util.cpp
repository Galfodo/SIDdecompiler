
#include "Util.h"
#include "OpcodeDefs.h"

#include <stdio.h>
#include <assert.h>

namespace SASM {

static Util::AssemblyError s_LastError;

Hue::Util::String Util::formatOpcode(byte* data, word address, bool includeAddress, bool includeHex, bool includeDisasm) {
  assert(data);
  OpcodeDef const* def = OpcodeDefs::getOpcodeDef(*data);
  int operandSize = OpcodeDefs::getOperandSize(def->m_AddrMode);
  Hue::Util::String sResult;
  if (includeAddress) {
    sResult.appendf(",%04x  ", address);
  } else {
    sResult.appendf("         ");
  }
  if (includeHex) {
    sResult.appendf("%02x ", (int)data[0]);
    if (operandSize > 0) {
      sResult.appendf("%02x ", (int)data[1]);
      if (operandSize > 1) {
        sResult.appendf("%02x ", (int)data[2]);
      } else {
        sResult.appendf("   ");
      }
    } else {
      sResult.appendf("      ");
    }
    sResult.append("    ");
  } else {
    sResult.appendf("             ");
  }
  if (includeDisasm) {
    sResult.appendf("%s ", def->m_Name);
    switch (def->m_AddrMode) {
    case AddrMode::XIND:
    case AddrMode::INDY:
    case AddrMode::IND:
      sResult.append('(');
      break;
    }
    if (def->m_AddrMode == AddrMode::IMM) {
      sResult.append('#');
    }
    if (def->m_AddrMode == AddrMode::REL) {
      int target = calculateBranchTarget(address, data[1]);
      sResult.appendf("$%04x", target);
    } else if (operandSize == 1) {
      sResult.appendf("$%02x", (int)data[1]);
    } else if (operandSize == 2) {
      sResult.appendf("$%04x", (int)data[1] | (data[2] << 8));
    }
    switch (def->m_AddrMode) {
    case AddrMode::ABSX:
    case AddrMode::XIND:
    case AddrMode::ZPX:
      sResult.append(",X");
      break;
    case AddrMode::ABSY:
    case AddrMode::ZPY:
      sResult.append(",Y");
      break;
    }
    switch (def->m_AddrMode) {
    case AddrMode::INDY:
    case AddrMode::XIND:
    case AddrMode::IND:
      sResult.append(")");
      if (def->m_AddrMode == AddrMode::INDY) {
        sResult.append(",Y");
      }
      break;
    }
  }
  sResult.left_justify(40);
  return sResult;
}

Hue::Util::String Util::formatMem(byte* data, word address, int columns) {
  assert(columns);
  assert(columns % 4 == 0);
  int* buf = new int[columns];
  Hue::Util::String sLine;
  sLine.printf(":%04x ", address);
  for (int i = 0; i < columns; ++i) {
    buf[i] = data[i];
    address = (address + 1) & 0xffff;
  }
  for (int c = 0; c < columns / 4; ++c) {
    sLine.appendf("%02x %02x %02x %02x  ", buf[c*4 + 0], buf[c*4 + 1], buf[c*4 + 2], buf[c*4 + 3]);
  }
  for (int i = 0; i < columns; ++i) {
    if (buf[i] >= 0x20 && buf[i] < 0x80) {
      sLine.append((char)buf[i]);
    } else {
      sLine.append('.');
    }
  }
  delete [] buf;
  return sLine;
}

int Util::calculateBranchOperand(int pc, int target) {
  pc += 2;
  int op = target - pc;
  if (target < pc) {
    op += 0x100;
    if (op < 0x80) {
      return -1;
    }
  } else if (op > 0x7f) {
    return -1;
  }
  assert(calculateBranchTarget((word)pc-2, (byte)op) == target);
  return op;
}

Util::AssemblyError Util::assembleSimpleLine(std::vector<byte>& output, const char* text, int current_pc) {
  output.clear();
  Hue::Util::String args;
  Hue::Util::String sOp;
  AssemblyError retcode               = OK;
  OpcodeDef* def                      = NULL;
  int operand                         = 0;
  AddrMode mode                       = AddrMode::IMPL;
  std::vector<OpcodeDef*> const* defs = NULL;
  auto sText = Hue::Util::String(text).trim().toupper();
  if (sText.empty()) {
    retcode = NO_INPUT;
    goto end;
  }
  if (sText.length() < 3) {
    retcode = SYNTAX_ERROR;
    goto end;
  }
  sOp = sText.substring(0, 3);
  defs = OpcodeDefs::checkIsInstruction(sOp);
  if (defs == NULL) {
    retcode = UNKNOWN_OPCODE;
    goto end;
  }
  args = sText.substring(3).trim().replace(" ", "").replace("$", "");
  mode = AddrMode::IMPL;
  if        (args.starts_with("(") && args.ends_with(",X)")) {
    mode = AddrMode::XIND;
  } else if (args.starts_with("(") && args.ends_with(")")) {
    mode = AddrMode::IND;
  } else if (args.starts_with("(") && args.ends_with("),Y")) {
    mode = AddrMode::INDY;
  } else                          if (args.ends_with(",Y")) {
    if (args.length() > 4) {
      mode = AddrMode::ABSY;
    } else {
      mode = AddrMode::ZPY;
    }
  } else                          if (args.ends_with(",X")) {
    if (args.length() > 4) {
      mode = AddrMode::ABSX;
    } else {
      mode = AddrMode::ZPX;
    }
  } else if (args.starts_with("#")) {
    mode = AddrMode::IMM;
  } else if (args.empty()) {
    mode = AddrMode::IMPL;
  } else {
    if (OpcodeDefs::isBranchInstruction(defs->at(0)->m_Op)) {
      mode = AddrMode::REL;
    } else if (args.length() <= 2) {
      mode = AddrMode::ZP;
    } else {
      mode = AddrMode::ABS;
    }
  }
  args.replace("(", "").replace(")", "").replace(",", "").replace("X", "").replace("Y", "").replace("#", "");
  if (mode == AddrMode::IMPL) {
    def = OpcodeDefs::getOpcodeDefForAddressingMode(defs, mode);
    output.push_back(def->m_Opcode & 0xff);
  } else {
    if (sscanf(args.c_str(), "%x", (unsigned int*)&operand) != 1) {
      retcode = SYNTAX_ERROR;
      goto end;
    }
    def = OpcodeDefs::getOpcodeDefForAddressingMode(defs, mode);
    if (def == NULL) {
      switch (mode)
      {
      case SASM::AddrMode::ZP:
        mode = AddrMode::ABS;
        break;
      case SASM::AddrMode::ZPX:
        mode = AddrMode::ABSX;
        break;
      case SASM::AddrMode::ZPY:
        mode = AddrMode::ABSY;
        break;
      }
      def = OpcodeDefs::getOpcodeDefForAddressingMode(defs, mode);
    }
    if (def == NULL) {
      retcode = ILLEGAL_ADDRESSING_MODE;
      goto end;
    }
    output.push_back(def->m_Opcode & 0xff);
    switch (mode) {
    case AddrMode::IMPL:
      break;
    case AddrMode::REL:
      {
        int op = Util::calculateBranchOperand(current_pc, operand);
        if (op < 0) {
          retcode = BRANCH_OUT_OF_RANGE;
          goto end;
        }
        output.push_back(op & 0xff);
      }
      break;
    case AddrMode::ABS:
    case AddrMode::ABSX:
    case AddrMode::ABSY:
    case AddrMode::IND:
      if (operand > 0xffff) {
        retcode = VALUE_OUT_OF_RANGE;
        goto end;
      }
      output.push_back(operand & 0xff);
      output.push_back((operand >> 8) & 0xff);
      break;
    default:
      if (operand > 0xff) {
        retcode = VALUE_OUT_OF_RANGE;
        goto end;
      }
      output.push_back(operand & 0xff);
    }
  }
end:
  if (retcode != OK) {
    output.clear();
    s_LastError = retcode;
  }
  return retcode;
}

Hue::Util::String Util::getLastError() {
  Hue::Util::String sResult;
  switch (s_LastError) {
  case AssemblyError::BRANCH_OUT_OF_RANGE:
    sResult = "Branch out of range";
    break;
  case AssemblyError::ILLEGAL_ADDRESSING_MODE:
    sResult = "Illegal addressing mode";
    break;
  case AssemblyError::NO_INPUT:
    sResult = "No input";
    break;
  case AssemblyError::SYNTAX_ERROR:
    sResult = "Syntax error";
    break;
  case AssemblyError::UNKNOWN_OPCODE:
    sResult = "Unknown opcode";
    break;
  case AssemblyError::VALUE_OUT_OF_RANGE:
    sResult = "Value out of range";
    break;
  }
  return sResult;
}

} // namespace SASM
