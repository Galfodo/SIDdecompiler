
#include "OpcodeDefs.h"
#include <vector>
#include <assert.h>
#include <ctype.h>

namespace SASM {

OpcodeDef OpcodeDefs::s_Opcodes[] = {
  OpcodeDef("brk", AddrMode::IMPL, Op::BRK, 7),
  OpcodeDef("ora", AddrMode::XIND, Op::ORA, 6),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 0),
  OpcodeDef("slo", AddrMode::XIND, Op::SLO, 8),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 3),
  OpcodeDef("ora", AddrMode::ZP, Op::ORA,   3),
  OpcodeDef("asl", AddrMode::ZP, Op::ASL,   5),
  OpcodeDef("slo", AddrMode::ZP, Op::SLO,   5),
  OpcodeDef("php", AddrMode::IMPL, Op::PHP, 3),
  OpcodeDef("ora", AddrMode::IMM, Op::ORA,  2),
  OpcodeDef("asl", AddrMode::IMPL, Op::ASL, 2),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 2),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4),
  OpcodeDef("ora", AddrMode::ABS, Op::ORA,  4),
  OpcodeDef("asl", AddrMode::ABS, Op::ASL,  6),
  OpcodeDef("slo", AddrMode::ABS, Op::SLO,  6),
  OpcodeDef("bpl", AddrMode::REL, Op::BPL,  2, 1),
  OpcodeDef("ora", AddrMode::INDY, Op::ORA, 5, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 0),
  OpcodeDef("slo", AddrMode::INDY, Op::SLO, 8),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4),
  OpcodeDef("ora", AddrMode::ZPX, Op::ORA,  4),
  OpcodeDef("asl", AddrMode::ZPX, Op::ASL,  6),
  OpcodeDef("slo", AddrMode::ZPX, Op::SLO,  6),
  OpcodeDef("clc", AddrMode::IMPL, Op::CLC, 2),
  OpcodeDef("ora", AddrMode::ABSY, Op::ORA, 4, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 2),
  OpcodeDef("slo", AddrMode::ABSY, Op::SLO, 7),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4, 1),
  OpcodeDef("ora", AddrMode::ABSX, Op::ORA, 4, 1),
  OpcodeDef("asl", AddrMode::ABSX, Op::ASL, 7),
  OpcodeDef("slo", AddrMode::ABSX, Op::SLO, 7),
  OpcodeDef("jsr", AddrMode::ABS, Op::JSR,  6),
  OpcodeDef("and", AddrMode::XIND, Op::AND, 6),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 0),
  OpcodeDef("rla", AddrMode::XIND, Op::RLA, 8),
  OpcodeDef("bit", AddrMode::ZP, Op::BIT,   3),
  OpcodeDef("and", AddrMode::ZP, Op::AND,   3),
  OpcodeDef("rol", AddrMode::ZP, Op::ROL,   5),
  OpcodeDef("rla", AddrMode::ZP, Op::RLA,   5),
  OpcodeDef("plp", AddrMode::IMPL, Op::PLP, 4),
  OpcodeDef("and", AddrMode::IMM, Op::AND,  2),
  OpcodeDef("rol", AddrMode::IMPL, Op::ROL, 2),
  OpcodeDef("anc", AddrMode::IMM, Op::ANC,  2),
  OpcodeDef("bit", AddrMode::ABS, Op::BIT,  4),
  OpcodeDef("and", AddrMode::ABS, Op::AND,  4),
  OpcodeDef("rol", AddrMode::ABS, Op::ROL,  6),
  OpcodeDef("rla", AddrMode::ABS, Op::RLA,  6),
  OpcodeDef("bmi", AddrMode::REL, Op::BMI,  2, 1),
  OpcodeDef("and", AddrMode::INDY, Op::AND, 5, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 0),
  OpcodeDef("rla", AddrMode::INDY, Op::RLA, 8),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4),
  OpcodeDef("and", AddrMode::ZPX, Op::AND,  4),
  OpcodeDef("rol", AddrMode::ZPX, Op::ROL,  6),
  OpcodeDef("rla", AddrMode::ZPX, Op::RLA,  6),
  OpcodeDef("sec", AddrMode::IMPL, Op::SEC, 2),
  OpcodeDef("and", AddrMode::ABSY, Op::AND, 4, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 2),
  OpcodeDef("rla", AddrMode::ABSY, Op::RLA, 7),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4, 1),
  OpcodeDef("and", AddrMode::ABSX, Op::AND, 4, 1),
  OpcodeDef("rol", AddrMode::ABSX, Op::ROL, 7),
  OpcodeDef("rla", AddrMode::ABSX, Op::RLA, 7),
  OpcodeDef("rti", AddrMode::IMPL, Op::RTI, 6),
  OpcodeDef("eor", AddrMode::XIND, Op::EOR, 6),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 0),
  OpcodeDef("sre", AddrMode::XIND, Op::SRE, 8),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 3),
  OpcodeDef("eor", AddrMode::ZP, Op::EOR,   3),
  OpcodeDef("lsr", AddrMode::ZP, Op::LSR,   5),
  OpcodeDef("sre", AddrMode::ZP, Op::SRE,   5),
  OpcodeDef("pha", AddrMode::IMPL, Op::PHA, 3),
  OpcodeDef("eor", AddrMode::IMM, Op::EOR,  2),
  OpcodeDef("lsr", AddrMode::IMPL, Op::LSR, 2),
  OpcodeDef("alr", AddrMode::IMM, Op::ALR,  2),
  OpcodeDef("jmp", AddrMode::ABS, Op::JMP,  3),
  OpcodeDef("eor", AddrMode::ABS, Op::EOR,  4),
  OpcodeDef("lsr", AddrMode::ABS, Op::LSR,  6),
  OpcodeDef("sre", AddrMode::ABS, Op::SRE,  6),
  OpcodeDef("bvc", AddrMode::REL, Op::BVC,  2, 1),
  OpcodeDef("eor", AddrMode::INDY, Op::EOR, 5, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 0),
  OpcodeDef("sre", AddrMode::INDY, Op::SRE, 8),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4),
  OpcodeDef("eor", AddrMode::ZPX, Op::EOR,  4),
  OpcodeDef("lsr", AddrMode::ZPX, Op::LSR,  6),
  OpcodeDef("sre", AddrMode::ZPX, Op::SRE,  6),
  OpcodeDef("cli", AddrMode::IMPL, Op::CLI, 2),
  OpcodeDef("eor", AddrMode::ABSY, Op::EOR, 4, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 2),
  OpcodeDef("sre", AddrMode::ABSY, Op::SRE, 7),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4, 1),
  OpcodeDef("eor", AddrMode::ABSX, Op::EOR, 4, 1),
  OpcodeDef("lsr", AddrMode::ABSX, Op::LSR, 7),
  OpcodeDef("sre", AddrMode::ABSX, Op::SRE, 7),
  OpcodeDef("rts", AddrMode::IMPL, Op::RTS, 6),
  OpcodeDef("adc", AddrMode::XIND, Op::ADC, 6),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 0),
  OpcodeDef("rra", AddrMode::XIND, Op::RRA, 8),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 3),
  OpcodeDef("adc", AddrMode::ZP, Op::ADC,   3),
  OpcodeDef("ror", AddrMode::ZP, Op::ROR,   5),
  OpcodeDef("rra", AddrMode::ZP, Op::RRA,   5),
  OpcodeDef("pla", AddrMode::IMPL, Op::PLA, 4),
  OpcodeDef("adc", AddrMode::IMM, Op::ADC,  2),
  OpcodeDef("ror", AddrMode::IMPL, Op::ROR, 2),
  OpcodeDef("arr", AddrMode::IMM, Op::ARR,  2),
  OpcodeDef("jmp", AddrMode::IND, Op::JMP,  5),
  OpcodeDef("adc", AddrMode::ABS, Op::ADC,  4),
  OpcodeDef("ror", AddrMode::ABS, Op::ROR,  6),
  OpcodeDef("rra", AddrMode::ABS, Op::RRA,  6),
  OpcodeDef("bvs", AddrMode::REL, Op::BVS,  2, 1),
  OpcodeDef("adc", AddrMode::INDY, Op::ADC, 5, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 0),
  OpcodeDef("rra", AddrMode::INDY, Op::RRA, 8),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4),
  OpcodeDef("adc", AddrMode::ZPX, Op::ADC,  4),
  OpcodeDef("ror", AddrMode::ZPX, Op::ROR,  6),
  OpcodeDef("rra", AddrMode::ZPX, Op::RRA,  6),
  OpcodeDef("sei", AddrMode::IMPL, Op::SEI, 2),
  OpcodeDef("adc", AddrMode::ABSY, Op::ADC, 4, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 2),
  OpcodeDef("rra", AddrMode::ABSY, Op::RRA, 7),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4, 1),
  OpcodeDef("adc", AddrMode::ABSX, Op::ADC, 4, 1),
  OpcodeDef("ror", AddrMode::ABSX, Op::ROR, 7),
  OpcodeDef("rra", AddrMode::ABSX, Op::RRA, 7),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 2),
  OpcodeDef("sta", AddrMode::XIND, Op::STA, 6),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 2),
  OpcodeDef("sax", AddrMode::XIND, Op::SAX, 6),
  OpcodeDef("sty", AddrMode::ZP, Op::STY,   3),
  OpcodeDef("sta", AddrMode::ZP, Op::STA,   3),
  OpcodeDef("stx", AddrMode::ZP, Op::STX,   3),
  OpcodeDef("sax", AddrMode::ZP, Op::SAX,   3),
  OpcodeDef("dey", AddrMode::IMPL, Op::DEY, 2),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 2),
  OpcodeDef("txa", AddrMode::IMPL, Op::TXA, 2),
  OpcodeDef("xaa", AddrMode::IMM, Op::XAA,  2),
  OpcodeDef("sty", AddrMode::ABS, Op::STY,  4),
  OpcodeDef("sta", AddrMode::ABS, Op::STA,  4),
  OpcodeDef("stx", AddrMode::ABS, Op::STX,  4),
  OpcodeDef("sax", AddrMode::ABS, Op::SAX,  4),
  OpcodeDef("bcc", AddrMode::REL, Op::BCC,  2, 1),
  OpcodeDef("sta", AddrMode::INDY, Op::STA, 6),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 0),
  OpcodeDef("ahx", AddrMode::INDY, Op::AHX, 6),
  OpcodeDef("sty", AddrMode::ZPX, Op::STY,  4),
  OpcodeDef("sta", AddrMode::ZPX, Op::STA,  4),
  OpcodeDef("stx", AddrMode::ZPY, Op::STX,  4),
  OpcodeDef("sax", AddrMode::ZPY, Op::SAX,  4),
  OpcodeDef("tya", AddrMode::IMPL, Op::TYA, 2),
  OpcodeDef("sta", AddrMode::ABSY, Op::STA, 5),
  OpcodeDef("txs", AddrMode::IMPL, Op::TXS, 2),
  OpcodeDef("tas", AddrMode::ABSY, Op::TAS, 5),
  OpcodeDef("shy", AddrMode::ABSX, Op::SHY, 5),
  OpcodeDef("sta", AddrMode::ABSX, Op::STA, 5),
  OpcodeDef("shx", AddrMode::ABSY, Op::SHX, 5),
  OpcodeDef("ahx", AddrMode::ABSY, Op::AHX, 5),
  OpcodeDef("ldy", AddrMode::IMM, Op::LDY,  2),
  OpcodeDef("lda", AddrMode::XIND, Op::LDA, 6),
  OpcodeDef("ldx", AddrMode::IMM, Op::LDX,  2),
  OpcodeDef("lax", AddrMode::XIND, Op::LAX, 6),
  OpcodeDef("ldy", AddrMode::ZP, Op::LDY,   3),
  OpcodeDef("lda", AddrMode::ZP, Op::LDA,   3),
  OpcodeDef("ldx", AddrMode::ZP, Op::LDX,   3),
  OpcodeDef("lax", AddrMode::ZP, Op::LAX,   3),
  OpcodeDef("tay", AddrMode::IMPL, Op::TAY, 2),
  OpcodeDef("lda", AddrMode::IMM, Op::LDA,  2),
  OpcodeDef("tax", AddrMode::IMPL, Op::TAX, 2),
  OpcodeDef("lax", AddrMode::IMM, Op::LAX,  2),
  OpcodeDef("ldy", AddrMode::ABS, Op::LDY,  4),
  OpcodeDef("lda", AddrMode::ABS, Op::LDA,  4),
  OpcodeDef("ldx", AddrMode::ABS, Op::LDX,  4),
  OpcodeDef("lax", AddrMode::ABS, Op::LAX,  4),
  OpcodeDef("bcs", AddrMode::REL, Op::BCS,  2, 1),
  OpcodeDef("lda", AddrMode::INDY, Op::LDA, 5, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 0),
  OpcodeDef("lax", AddrMode::INDY, Op::LAX, 5, 1),
  OpcodeDef("ldy", AddrMode::ZPX, Op::LDY,  4),
  OpcodeDef("lda", AddrMode::ZPX, Op::LDA,  4),
  OpcodeDef("ldx", AddrMode::ZPY, Op::LDX,  4),
  OpcodeDef("lax", AddrMode::ZPY, Op::LAX,  4),
  OpcodeDef("clv", AddrMode::IMPL, Op::CLV, 2),
  OpcodeDef("lda", AddrMode::ABSY, Op::LDA, 4, 1),
  OpcodeDef("tsx", AddrMode::IMPL, Op::TSX, 2),
  OpcodeDef("las", AddrMode::ABSY, Op::LAS, 4, 1),
  OpcodeDef("ldy", AddrMode::ABSX, Op::LDY, 4, 1),
  OpcodeDef("lda", AddrMode::ABSX, Op::LDA, 4, 1),
  OpcodeDef("ldx", AddrMode::ABSY, Op::LDX, 4, 1),
  OpcodeDef("lax", AddrMode::ABSY, Op::LAX, 4, 1),
  OpcodeDef("cpy", AddrMode::IMM, Op::CPY,  2),
  OpcodeDef("cmp", AddrMode::XIND, Op::CMP, 6),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 2),
  OpcodeDef("dcp", AddrMode::XIND, Op::DCP, 8),
  OpcodeDef("cpy", AddrMode::ZP, Op::CPY,   3),
  OpcodeDef("cmp", AddrMode::ZP, Op::CMP,   3),
  OpcodeDef("dec", AddrMode::ZP, Op::DEC,   5),
  OpcodeDef("dcp", AddrMode::ZP, Op::DCP,   5),
  OpcodeDef("iny", AddrMode::IMPL, Op::INY, 2),
  OpcodeDef("cmp", AddrMode::IMM, Op::CMP,  2),
  OpcodeDef("dex", AddrMode::IMPL, Op::DEX, 2),
  OpcodeDef("axs", AddrMode::IMM, Op::AXS,  2),
  OpcodeDef("cpy", AddrMode::ABS, Op::CPY,  4),
  OpcodeDef("cmp", AddrMode::ABS, Op::CMP,  4),
  OpcodeDef("dec", AddrMode::ABS, Op::DEC,  6),
  OpcodeDef("dcp", AddrMode::ABS, Op::DCP,  6),
  OpcodeDef("bne", AddrMode::REL, Op::BNE,  2, 1),
  OpcodeDef("cmp", AddrMode::INDY, Op::CMP, 5, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 0),
  OpcodeDef("dcp", AddrMode::INDY, Op::DCP, 8),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4),
  OpcodeDef("cmp", AddrMode::ZPX, Op::CMP,  4),
  OpcodeDef("dec", AddrMode::ZPX, Op::DEC,  6),
  OpcodeDef("dcp", AddrMode::ZPX, Op::DCP,  6),
  OpcodeDef("cld", AddrMode::IMPL, Op::CLD, 2),
  OpcodeDef("cmp", AddrMode::ABSY, Op::CMP, 4, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 2),
  OpcodeDef("dcp", AddrMode::ABSY, Op::DCP, 7),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4, 1),
  OpcodeDef("cmp", AddrMode::ABSX, Op::CMP, 4, 1),
  OpcodeDef("dec", AddrMode::ABSX, Op::DEC, 7),
  OpcodeDef("dcp", AddrMode::ABSX, Op::DCP, 7),
  OpcodeDef("cpx", AddrMode::IMM, Op::CPX,  2),
  OpcodeDef("sbc", AddrMode::XIND, Op::SBC, 6),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 2),
  OpcodeDef("isc", AddrMode::XIND, Op::ISC, 8),
  OpcodeDef("cpx", AddrMode::ZP, Op::CPX,   3),
  OpcodeDef("sbc", AddrMode::ZP, Op::SBC,   3),
  OpcodeDef("inc", AddrMode::ZP, Op::INC,   5),
  OpcodeDef("isc", AddrMode::ZP, Op::ISC,   5),
  OpcodeDef("inx", AddrMode::IMPL, Op::INX, 2),
  OpcodeDef("sbc", AddrMode::IMM, Op::SBC,  2),
  OpcodeDef("nop", AddrMode::IMPL, Op::NOP, 2),
  OpcodeDef("sbc", AddrMode::IMM, Op::SBC,  2),
  OpcodeDef("cpx", AddrMode::ABS, Op::CPX,  4),
  OpcodeDef("sbc", AddrMode::ABS, Op::SBC,  4),
  OpcodeDef("inc", AddrMode::ABS, Op::INC,  6),
  OpcodeDef("isc", AddrMode::ABS, Op::ISC,  6),
  OpcodeDef("beq", AddrMode::REL, Op::BEQ,  2, 1),
  OpcodeDef("sbc", AddrMode::INDY, Op::SBC, 5, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 0),
  OpcodeDef("isc", AddrMode::INDY, Op::ISC, 8),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4),
  OpcodeDef("sbc", AddrMode::ZPX, Op::SBC,  4),
  OpcodeDef("inc", AddrMode::ZPX, Op::INC,  6),
  OpcodeDef("isc", AddrMode::ZPX, Op::ISC,  6),
  OpcodeDef("sed", AddrMode::IMPL, Op::SED, 2),
  OpcodeDef("sbc", AddrMode::ABSY, Op::SBC, 4, 1),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 2),
  OpcodeDef("isc", AddrMode::ABSY, Op::ISC, 7),
  OpcodeDef("ill", AddrMode::IMPL, Op::ILL, 4, 1),
  OpcodeDef("sbc", AddrMode::ABSX, Op::SBC, 4, 1),
  OpcodeDef("inc", AddrMode::ABSX, Op::INC, 7),
  OpcodeDef("isc", AddrMode::ABSX, Op::ISC, 7)
};

struct TrieNode {
  TrieNode() : m_OpcodeDefs(NULL) {
    memset(m_Children, 0, sizeof(m_Children));
  }

  ~TrieNode() {
    for (int i = 0; i < sizeof(m_Children) / sizeof(m_Children[0]); ++i) {
      delete m_Children[i];
    }
    delete m_OpcodeDefs;
  }

  std::vector<OpcodeDef*> const* findOpcodeDefs(const char* name, int nameindex) {
    if (m_OpcodeDefs) {
      return m_OpcodeDefs;
    } else {
      char i = name[nameindex];
      if (m_Children[i]) {
        return m_Children[i]->findOpcodeDefs(name, nameindex + 1);
      }
      return NULL;
    }
  }

  TrieNode* m_Children[256];
  std::vector<OpcodeDef*>* m_OpcodeDefs;

protected:
  void addNode(const char* name, int nameindex, OpcodeDef* opcodedef) {
    if (name[nameindex] == 0) {
      if (this->m_OpcodeDefs == NULL) {
        this->m_OpcodeDefs = new std::vector<OpcodeDef*>();
      }
      this->m_OpcodeDefs->push_back(opcodedef);
    } else {
      char charindex1 = name[nameindex];      // Opcode name assumed to be upper case
      char charindex2 = tolower(charindex1);  // Register for lower case also
      if (m_Children[charindex1] == NULL) {
        assert(m_Children[charindex2] == NULL);
        m_Children[charindex1] = new TrieNode();
        m_Children[charindex2] = new TrieNode(); // We don't really need 2 nodes, but it makes for easier cleanup
      }
      m_Children[charindex1]->addNode(name, nameindex + 1, opcodedef);
      m_Children[charindex2]->addNode(name, nameindex + 1, opcodedef);
    }
  }

};

struct TrieNodeRoot : public TrieNode {
  TrieNodeRoot() {
    for (int i = 0; i < 256; ++i) {
      OpcodeDefs::s_Opcodes[i].m_Opcode = i;
      addNode(OpcodeDefs::s_Opcodes[i].m_Name, 0, &OpcodeDefs::s_Opcodes[i]);
    }
  }

  std::vector<OpcodeDef*> const* findOpcodeDefs(const char* token) {
    return TrieNode::findOpcodeDefs(token, 0);
  }
};

static TrieNodeRoot
  s_Root;

static bool
  s_isOpSizeLookupInitialized;

static byte
  s_OperandSize[256];

OpcodeDef::OpcodeDef(const char* name, AddrMode addrmode, Op op, int cycles, int page_crossing_penalty) :
  m_Name(name),
  m_AddrMode(addrmode),
  m_Op(op),
  m_Cycles(cycles),
  m_PageCrossingPenalty(page_crossing_penalty),
  m_Opcode(0)
{
}

int OpcodeDefs::getOperandSize(AddrMode addrmode) {
  switch (addrmode)
  {
    case AddrMode::ABS:
    case AddrMode::ABSX:
    case AddrMode::ABSY:
    case AddrMode::IND:
      return 2;
    case AddrMode::IMM:
    case AddrMode::ZP:
    case AddrMode::ZPX:
    case AddrMode::ZPY:
    case AddrMode::INDY:
    case AddrMode::XIND:
    case AddrMode::REL:
      return 1;
    default:
      return 0;
  }
}

int OpcodeDefs::getOperandSize(OpcodeDef* def) {
  assert(def);
  return getOperandSize(def->m_AddrMode);
}

int OpcodeDefs::getOperandSize(byte opcode) {
  if (!s_isOpSizeLookupInitialized) {
    for (int i = 0; i < 256; ++i) {
      auto def = OpcodeDefs::getOpcodeDef((byte)i);
      s_OperandSize[i] = OpcodeDefs::getOperandSize(def->m_AddrMode);
    }
    s_isOpSizeLookupInitialized = true;
  }
  return s_OperandSize[opcode];
}

std::vector<OpcodeDef*> const* OpcodeDefs::checkIsInstruction(Token const& token) {
  if (token.length() == 3) {
    char name[4];
    int i = 0;
    name[i] = token.m_pzTokenStart[i]; i++;
    name[i] = token.m_pzTokenStart[i]; i++;
    name[i] = token.m_pzTokenStart[i]; i++;
    name[i] = '\0';
    return s_Root.findOpcodeDefs(name);
  } else {
    return NULL;
  }
}

std::vector<OpcodeDef*> const* OpcodeDefs::checkIsInstruction(Hue::Util::String const& str) {
  Token token(str.c_str());
  return checkIsInstruction(token);
}

bool OpcodeDefs::isBranchInstruction(Op op) {
  switch (op) {
  case Op::BPL:
  case Op::BMI:
  case Op::BVC:
  case Op::BVS:
  case Op::BCC:
  case Op::BCS:
  case Op::BNE:
  case Op::BEQ:
    return true;
  default:
    return false;
  }
}

OpcodeDef* OpcodeDefs::getOpcodeDefForAddressingMode(std::vector<OpcodeDef*> const* opcodedefs, AddrMode addrmode) {
  assert(opcodedefs);
  for (auto it = opcodedefs->begin(); it != opcodedefs->end(); ++it) {
    if ((*it)->m_AddrMode == addrmode) {
      return *it;
    }
  }
  return NULL;
}

} // namespace SASM
