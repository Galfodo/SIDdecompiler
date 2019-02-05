/////////////////////////////////////////////////////////////////////////////
// Profile guided disassembler
//
// KNOWN BUGS:
//
//
// FIXED:
//
// * ZP relocation not handled when addressing ABS (Ocean_Loader_1.sid Ocean_Loader_3.sid)
// * Branch outside relocation range (Thing_On_A_Spring.sid)

#include "Disassembler.h"
#include "OpcodeDefs.h"
#include "Util.h"
#include "Profiling.h"

namespace SASM {

Disassembler::Region::Region(int addr, int size, RegionType type) : m_Address(addr), m_Size(size), m_Type(type) {
}

Disassembler::Label::Label(int addr, const char* name, bool isAlias, bool executeableAddress) : m_Address(addr), m_Name(name), m_IsAlias(isAlias), m_ExecutableAddressHint(executeableAddress), m_IsGenerated(false) {
}

Disassembler::Line::Line(int addr, Hue::Util::String const& instr, Hue::Util::String const& op, bool referenced) : m_Address(addr), m_Instruction(instr), m_Operand(op), m_Referenced(referenced) {
}

Disassembler::AddressTable::AddressTable(const char* label_prefix, int loaddr, int hiaddr, int size) : m_LabelPrefix(label_prefix), m_LoAddr(loaddr), m_HiAddr(hiaddr), m_Size(size), m_Distance(1) {
}

Disassembler::AddressTable::AddressTable(const char* label_prefix, int wordaddr, int size) : m_LabelPrefix(label_prefix), m_LoAddr(wordaddr), m_HiAddr(wordaddr + 1), m_Size(size), m_Distance(2) {
}

Disassembler::Disassembler(byte* data, int addr, int size, bool commentUnusedData, bool commentUnusedCode) : m_Data(data), m_Address(addr), m_Size(size), m_AllowIllegals(false), m_UnknownAlwaysData(true), m_CommentUnusedData(commentUnusedData), m_CommentUnusedCode(commentUnusedCode), m_RelocZP(false), m_RelocZPaddr(0), m_IgnoreIOArea(true) {
}

Disassembler::~Disassembler() {
}

static Disassembler::RegionType getAddressRegionType(MemAccessMap* map, int addr) {
  int flags = map->getAccessType(addr);
  if (flags & (MemAccessMap::EXECUTE | MemAccessMap::OPERAND)) {
    return Disassembler::CODE;
  } else if (flags == MemAccessMap::UNTOUCHED) {
    return Disassembler::UNKNOWN;
  }
  return Disassembler::DATA;
}

void Disassembler::parseMemAccessMap(MemAccessMap* map) {
  Region r(m_Address, 0, getAddressRegionType(map, m_Address));
  for (int addr = m_Address; addr < m_Address + m_Size; ++addr) {
    if ((map->getAccessType(addr) & (MemAccessMap::OPERAND | MemAccessMap::WRITE)) == (MemAccessMap::OPERAND | MemAccessMap::WRITE)) {
      if (map->getAccessType(addr-1) & MemAccessMap::EXECUTE) {
        auto lbl = getOrCreateLabel(addr-1, defaultLabel(addr).c_str(), true);
        addOffsetAlias(lbl, 1);
      } else {
        auto lbl = getOrCreateLabel(addr-2, defaultLabel(addr).c_str(), true);
        addOffsetAlias(lbl, 2);
      }
    }
    auto type = getAddressRegionType(map, addr);
    if (type == r.m_Type) {
      ++r.m_Size;
    } else {
      addRegion(r);
      r = Region(addr, 1, getAddressRegionType(map, addr));
    }
  }
  if (r.m_Size > 0) {
    addRegion(r);
  }
}

Disassembler::Label* Disassembler::getLabel(int addr) {
  if (addr == 0x3700 || addr == 0x3800) {
    int debug = 0;
  }
  auto it = m_Labels.find(addr);
  if (it == m_Labels.end()) {
    return NULL;
  } else {
    return it->second;
  }
}

Disassembler::Label* Disassembler::getOrCreateLabel(int addr, const char* name, bool executeableAddress) {
  //if (addr == 0x8110) {
  //  int debug = 0;
  //}
  auto lbl = getLabel(addr);
  return lbl ? lbl : addLabel(addr, name, executeableAddress);
}


Disassembler::Label* Disassembler::addLabel(int addr, const char* name, bool executeableAddress) {
  assert(name);
  assert(m_Labels.find(addr) == m_Labels.end());
  //if (addr >= 0xd000 && addr < 0xe000) {
  //  int debug = 0;
  //}
  auto lbl = new Label(addr, name, false, executeableAddress);
  m_Labels.insert(std::make_pair(addr, lbl));
  return lbl;
}

Disassembler::Label* Disassembler::addAlias(int addr, const char* alias) {
  assert(alias);
  auto it = m_Labels.find(addr);
  assert(it != m_Labels.end());
  auto lbl = new Label(addr, alias, false, it->second->m_ExecutableAddressHint);
  m_Alias.push_back(lbl);
  return lbl;
}

Disassembler::Label* Disassembler::addOffsetAlias(Label* alias, int offset) {
  assert(alias);
  auto lbl = getLabel(alias->m_Address + offset);
  if (lbl == NULL) {
    Hue::Util::String sLabel;
    sLabel.printf("%s%s%d", alias->m_Name.c_str(), (offset >= 0 ? "+" : ""), offset);
    int addr = alias->m_Address + offset;
    assert(m_Labels.find(addr) == m_Labels.end());
    lbl = new Label(addr, sLabel.c_str(), true, alias->m_ExecutableAddressHint);
    m_Labels.insert(std::make_pair(addr, lbl));
  }
  return lbl;
}

Hue::Util::String Disassembler::defaultLabel(int addr) {
  if (addr < 0x100) {
    return Hue::Util::String::static_printf("z%02x", addr);
  } else {
    return Hue::Util::String::static_printf("l%04x", addr);
  }
}

void Disassembler::formatAddress(Hue::Util::String& str, int addr, bool executeableAddress, bool forceLabel) {
  //if (addr == 0xa289) {
  //  int debug = 0;
  //}
  auto sDefaultLabel = defaultLabel(addr);
  if (forceLabel || isAddressInRange(addr)) {
    Label* lbl = getOrCreateLabel(addr, sDefaultLabel.c_str(), executeableAddress);
    str.append(lbl->m_Name);
  } else {
    Label* lbl = getLabel(addr);
    if (lbl) {
      str.append(lbl->m_Name);
    } else {
      str.appendf("$%04x", addr);
    }
  }
}

Hue::Util::String Disassembler::AddressTable::formatLabel(Disassembler& disasm, int addr, int index, bool isLowByte) {
  assert(index >= 0 && index < m_Size);
  Hue::Util::String sLabel;
  if (m_LabelPrefix.empty()) {
    auto lbl = disasm.getOrCreateLabel(addr, defaultLabel(addr).c_str(), true);
    sLabel.printf("%s%s", isLowByte ? "<" : ">", lbl->m_Name.c_str());
  } else {
    sLabel.printf("%s%s%02x", isLowByte ? "<" : ">", m_LabelPrefix.c_str(), index);
  }
  return sLabel;
}

Hue::Util::String Disassembler::AddressTable::formatLabel(Disassembler& disasm, int addr, int index) {
  assert(index >= 0 && index < m_Size);
  Hue::Util::String sLabel;
  if (m_LabelPrefix.empty()) {
    auto lbl = disasm.getOrCreateLabel(addr, defaultLabel(addr).c_str(), true);
    sLabel.printf("%s", lbl->m_Name.c_str());
  } else {
    sLabel.printf("%s%02x", m_LabelPrefix.c_str(), index);
  }
  return sLabel;
}

Disassembler::Line* Disassembler::formatOpcode(byte* data, int addr, bool referenced) {
  assert(data);
  OpcodeDef const* def = OpcodeDefs::getOpcodeDef(*data);
  bool defaultOperandAddressExecutable = false;
  int operandSize = OpcodeDefs::getOperandSize(def->m_AddrMode);
  Hue::Util::String sInstruction = def->m_Name;
  Hue::Util::String sOperand;
  if (def->m_Op == Op::JMP || def->m_Op == Op::JSR || OpcodeDefs::isBranchInstruction(def->m_Op)) {
    defaultOperandAddressExecutable = true;
  }
  switch (def->m_AddrMode) {
  case AddrMode::XIND:
  case AddrMode::INDY:
  case AddrMode::IND:
    sOperand.append('(');
    break;
  }
  if (operandSize > 0) {
    if (getLabel(addr + 1)) {
      auto lbl = getOrCreateLabel(addr, defaultLabel(addr).c_str(), defaultOperandAddressExecutable);
      addOffsetAlias(lbl, 1);
    }
    if (operandSize > 1) {
      if (getLabel(addr + 2)) {
        auto lbl = getOrCreateLabel(addr, defaultLabel(addr).c_str(), defaultOperandAddressExecutable);
        addOffsetAlias(lbl, 2);
      }
    }
  }
  if (def->m_AddrMode == AddrMode::IMM) {
    sOperand.append('#');
    AddressTable* tbl = NULL;
    int tblindex = 0;
    bool isLowByte = false;
    if (addressInRelocTable(&tbl, &tblindex, &isLowByte, addr + 1)) {
      int addrelem = getAddressFromTableAt(tbl, tblindex);
      sOperand.append(tbl->formatLabel(*this, addrelem, tblindex, isLowByte));
      //if (sOperand.starts_with("#>L00")) {
      //  int debug = 0;
      //}
      goto done;
    }
  }
  if (def->m_AddrMode == AddrMode::REL) {
    int target = Util::calculateBranchTarget(addr, data[1]);
    formatAddress(sOperand, target, true, true);
  } else if (operandSize == 1) {
    if (m_RelocZP && def->m_AddrMode != AddrMode::IMM && data[1] > 1) {
      m_ZPused.insert((int)data[1]);
      sOperand.appendf("z%02x", (int)data[1]);
    } else {
      sOperand.appendf("$%02x", (int)data[1]);
    }
  } else if (operandSize == 2) {
    int operand = data[1] | (data[2] << 8);
    formatAddress(sOperand, operand, def->m_Op == Op::JMP, m_RelocZP ? (operand >= 2 && operand < 0x100) : false);
  }
  switch (def->m_AddrMode) {
  case AddrMode::ABSX:
  case AddrMode::XIND:
  case AddrMode::ZPX:
    sOperand.append(",X");
    break;
  case AddrMode::ABSY:
  case AddrMode::ZPY:
    sOperand.append(",Y");
    break;
  }
  switch (def->m_AddrMode) {
  case AddrMode::INDY:
  case AddrMode::XIND:
  case AddrMode::IND:
    sOperand.append(")");
    if (def->m_AddrMode == AddrMode::INDY) {
      sOperand.append(",Y");
    }
    break;
  }
done:
  return new Line(addr, sInstruction, sOperand, referenced);
}

void Disassembler::addRegion(Region const& newregion) {
  assert(getRegion(newregion.m_Address) == NULL);
  auto it = m_Regions.begin();
  for (; it != m_Regions.end(); ++it) {
    auto r = *it;
    if (newregion.m_Address < r->m_Address) {
      break;
    }
  }
  if (it != m_Regions.end()) {
    assert(newregion.m_Address + newregion.m_Size <= (*it)->m_Address);
  }
  m_Regions.insert(it, new Region(newregion));
}

void Disassembler::addRegion(int addr, int size, RegionType type) {
  addRegion(Region(addr, size, type));
}


typedef std::vector<Disassembler::Region*> RegionList_t;
static RegionList_t::const_iterator
FindRegion(RegionList_t const& list, int addr) {
  for (auto it = list.begin(); it != list.end(); ++it) {
    auto r = *it;
    if (addr >= r->m_Address && addr < r->m_Address + r->m_Size) {
      return it;
    }
  }
  return list.end();
}

Disassembler::Region const* Disassembler::getRegion(int addr) {
  assert(addr >= m_Address && addr < m_Address + m_Size);
  auto it = FindRegion(m_Regions, addr);
  if (it != m_Regions.end()) {
    return *it;
  } else {
    return NULL;
  }
}

Disassembler::Region const* Disassembler::getNextRegion(int addr) {
  static Region dummy_end(0x10000, 0, Disassembler::UNKNOWN);
  assert(addr >= m_Address && addr < m_Address + m_Size);
  auto it = FindRegion(m_Regions, addr);
  if (it != m_Regions.end()) {
    ++it;
  }
  if (it != m_Regions.end()) {
    return *it;
  } else {
    return &dummy_end;
  }
}

Disassembler::Region const* Disassembler::getPrevRegion(int addr) {
  static Region dummy_begin(0x0000, 0, Disassembler::UNKNOWN);
  assert(addr >= m_Address && addr < m_Address + m_Size);
  auto it = FindRegion(m_Regions, addr);
  if (it == m_Regions.end() && it == m_Regions.begin()) {
    return &dummy_begin;
  }
  --it;
  return *it;
}

Disassembler::AddressTable* Disassembler::addAddressTable(const char* label_prefix, int addrlo, int addrhi, int length) {
  assert(length); // We could try to guess the length if length 0 is supplied...
  if (addrhi - addrlo == 1) {
    return addWordAddressTable(label_prefix, addrlo, length);
  } else {
    auto tbl = new AddressTable(label_prefix, addrlo, addrhi, length);
    m_AddressTables.push_back(tbl);
    return tbl;
  }
}

Disassembler::AddressTable* Disassembler::addWordAddressTable(const char* label_prefix, int addr, int length) {
  assert(length); // We could try to guess the length if length 0 is supplied...
  auto tbl = new AddressTable(label_prefix, addr, length);
  m_AddressTables.push_back(tbl);
  return tbl;
}

Disassembler::AddressTable* Disassembler::addRelocAddress(const char* label, int addrlo, int addrhi) {
  if (getByteAt(addrlo) == 0 && getByteAt(addrhi) == 0x37) {
    int debug = 0;
  }
  return addAddressTable(label, addrlo, addrhi, 1);
}

byte Disassembler::getByteAt(int addr) {
  assert(addr >= m_Address && addr < m_Address + m_Size);
  return m_Data[addr - m_Address];
}

int Disassembler::getAddressFromTableAt(AddressTable* tbl, int index) {
  assert(tbl);
  assert(index < tbl->m_Size);
  int loaddr = tbl->m_LoAddr + index * tbl->m_Distance;
  int hiaddr = tbl->m_HiAddr + index * tbl->m_Distance;
  int addr = getByteAt(loaddr) | (getByteAt(hiaddr) << 8);
  return addr;
}

  bool Disassembler::addressInRelocTable(AddressTable** table, int* index, bool* isLowbyte, int addr) {
  assert(table);
  assert(index);
  assert(isLowbyte);
  for (int i = 0; i < (int)m_AddressTables.size(); ++i) {
    auto tbl = m_AddressTables.at(i);
    if (tbl->m_Distance == 2) {
      if (addr >= tbl->m_LoAddr && addr < tbl->m_LoAddr + tbl->m_Size * 2) {
        *table = tbl;
        *index = (addr - tbl->m_LoAddr) / 2;
        *isLowbyte = (addr - tbl->m_LoAddr) & 1 ? false : true;
        return true;
      }
    } else {
      assert(tbl->m_Distance == 1);
      if (addr >= tbl->m_LoAddr && addr < tbl->m_LoAddr + tbl->m_Size) {
        *table = tbl;
        *index = addr - tbl->m_LoAddr;
        *isLowbyte = true;
        return true;
      } else if (addr >= tbl->m_HiAddr && addr < tbl->m_HiAddr + tbl->m_Size) {
        *table = tbl;
        *index = addr - tbl->m_HiAddr;
        *isLowbyte = false;
        return true;
      }
    }
  }
  return false;
}

void Disassembler::preprocessRegions() {
  if (m_Regions.empty()) {
    m_Regions.insert(m_Regions.begin(), new Region(m_Address, m_Size, CODE));
  }
  int addr = m_Address;
  //if (addr == 0x1b95) {
  //  int debug = 0;
  //}
  RegionType prevRegionType = UNKNOWN;
  bool isPrevBranch = false;
  while (addr < m_Address + m_Size) {
  }
}

void Disassembler::disassemble() {
  if (m_Regions.empty()) {
    m_Regions.insert(m_Regions.begin(), new Region(m_Address, m_Size, CODE));
  }
  int addr = m_Address;
  //if (addr == 0x1b95) {
  //  int debug = 0;
  //}
  RegionType prevRegionType = UNKNOWN;
  bool isPrevBranch = false;
  Region const* currentRegion = NULL;
  RegionType type = UNKNOWN;
  while (addr < m_Address + m_Size) {
    Region const* r = getRegion(addr);
    assert(r);
    RegionType realtype = r->m_Type;
    byte* data = m_Data + (addr - m_Address);
    if (r != currentRegion) {
      currentRegion = r;
      type = r->m_Type;
    }
    assert(currentRegion);
    OpcodeDef* def = OpcodeDefs::getOpcodeDef(*data);
    int operandSize = OpcodeDefs::getOperandSize(def->m_AddrMode);
    bool referenced = true;
    if (realtype == UNKNOWN) {
      referenced = false;
      //if (isPrevBranch) {
      //  type = CODE;
      //}
      if (prevRegionType == CODE) {
        type = CODE;
        auto l = getLabel(addr);
        if (l && !l->m_ExecutableAddressHint) {
          type = DATA;
        }
      }
      switch (def->m_Op) {
      case Op::AHX:
      case Op::ALR:
      case Op::ANC:
      case Op::ARR:
      case Op::AXS:
      case Op::BRK:
      case Op::DCP:
      case Op::ISC:
      case Op::ILL:
      case Op::KIL:
      case Op::LAS:
      case Op::RLA:
      case Op::RRA:
      case Op::SAX:
      case Op::SHX:
      case Op::SHY:
      case Op::SLO:
      case Op::SRE:
      case Op::TAS:
      case Op::XAA:
        type = DATA;
        break;
      case Op::LAX:
        // Check addressing mode?
        break;
      }
      if (m_UnknownAlwaysData) {
        type = DATA;
      }
    }
    prevRegionType = type;
    if (type == CODE) {
      isPrevBranch = OpcodeDefs::isBranchInstruction(def->m_Op);
      m_Lines.insert(std::make_pair(addr, formatOpcode(data, addr, referenced)));
      addr += operandSize + 1;
    } else {
      isPrevBranch = false;
      AddressTable* tbl = NULL;
      int tblindex = 0;
      bool isLowByte = false;
      if (addressInRelocTable(&tbl, &tblindex, &isLowByte, addr)) {
        int addrelem = getAddressFromTableAt(tbl, tblindex);
        auto sLabel = tbl->formatLabel(*this, addrelem, tblindex, isLowByte);
        m_Lines.insert(std::make_pair(addr, new Line(addr, ".byte", sLabel, referenced)));
      } else {
        m_Lines.insert(std::make_pair(addr, new Line(addr, ".byte", Hue::Util::String::static_printf("$%02x", (int)*data), referenced)));
      }
      ++addr;
    }
  }
}

struct TokenBufferer {
  TokenBufferer(Hue::Util::String& sTargetString, int tokens_per_line, const char* separator, bool tag_unreferenced) :
    m_TargetString(sTargetString), m_MaxTokens(tokens_per_line), m_Separator(separator), m_Referenced(true), m_TagUnreferenced(tag_unreferenced) {
  }

  ~TokenBufferer() {
    flush();
  }

  void add(Hue::Util::String const& token, bool referenced, const char* prefix) {
    if (m_Tokens.size() == 0) {
      m_Referenced = referenced;
    }
    if (m_Tokens.size() >= m_MaxTokens || (m_TagUnreferenced && referenced != m_Referenced)) {
      flush();
      m_Referenced = referenced;
      if (prefix) {
        m_TargetString.append(prefix);
      }
    }
    m_Tokens.append(token);
  }

  void flush() {
    if (m_Tokens.size()) {
      auto str = m_Tokens.join(m_Separator);
      if (m_TagUnreferenced && !m_Referenced) {
        str.left_justify(70).append("; Unreferenced data");
      }
      m_TargetString.append(str);
      m_TargetString.append("\n");
      m_Tokens.clear();
    }
  }

  bool empty() const {
    return m_Tokens.size() == 0;
  }

  Hue::Util::String&      m_TargetString;
  int                     m_MaxTokens;
  Hue::Util::String::List m_Tokens;
  const char*             m_Separator;
  bool                    m_Referenced;
  bool                    m_TagUnreferenced;
};

void Disassembler::setHeader(const char* header) {
  m_Header = header;
}

Hue::Util::String Disassembler::formatAddressUsingClosestLabel(int addr, bool force) {
  assert(addr >= 0x100);
  // Look for nearest defined label
  int distance = 0x10000;
  Label* found = 0;
  for (auto srchit = m_Labels.begin(); srchit != m_Labels.end(); ++srchit) {
    if (!srchit->second->m_IsAlias && (srchit->second->m_IsGenerated || force)) {
      int d = addr - srchit->second->m_Address;
      if (abs(d) < abs(distance)) {
        distance = d;
        found = srchit->second;
      }
    }
  }
  assert(found);
  if (distance == 0) {
    return found->m_Name;
  } else {
    return Hue::Util::String::static_printf("%s%s%d", found->m_Name.c_str(), distance >= 0 ? "+" : "", distance);
  }
}

Hue::Util::String Disassembler::dump(int relocaddr) {
  int tab_size = 16;
  int bytes_per_line = 8;
  relocaddr = relocaddr >= 0 ? relocaddr : m_Address;
  Hue::Util::String sText;
  sText.append(m_Header);
  if (m_Alias.size()) {
    for (auto it = m_Alias.begin(); it != m_Alias.end(); ++it) {
      auto lbl = this->getLabel((*it)->m_Address);
      assert(lbl);
      if (lbl) {
        sText.appendf("%s = %s\n", (*it)->m_Name.c_str(), lbl->m_Name.c_str());
      }
    }
  }
  if (!m_Header.contains("*=") && !m_Header.contains("* =")) {
    sText.append(Hue::Util::String("").left_justify(tab_size));
    sText.appendf(Hue::Util::String::static_printf("* = $%04x\n\n", relocaddr));
  }
  if (m_ZPused.size()) {
    int ZPprev = 0;
    for (auto it = m_ZPused.begin(); it != m_ZPused.end(); ++it) {
      Hue::Util::String sLine;
      sLine.left_justify(tab_size);
      if (it == m_ZPused.begin()) {
        ZPprev = *it;
        sLine.appendf("z%02x = $%02x\n", *it, m_RelocZPaddr >= 2 ? (m_RelocZPaddr & 0xff) : *it);
      } else {
        if (m_RelocZPaddr >= 2) {
          sLine.appendf("z%02x = z%02x + $01\n", *it, ZPprev);
        } else {
          sLine.appendf("z%02x = z%02x + $%02x\n", *it, ZPprev, *it - ZPprev);
        }
        ZPprev = *it;
      }
      sText.append(sLine);
    }
    sText.append("\n");
  }
  {
    TokenBufferer 
      byteTokens(sText, bytes_per_line, ", ", m_CommentUnusedData);

    for (auto it = m_Lines.begin(); it != m_Lines.end(); ++it) {
      Line const& line = *it->second;
      Hue::Util::String sLine;
      Label* lbl = getLabel(line.m_Address);
      if (lbl && !lbl->m_IsAlias) {
        byteTokens.flush();
        sLine.append(lbl->m_Name);
        lbl->m_IsGenerated = true;
      }
      if (line.m_Instruction == ".byte") {
        sLine.left_justify(tab_size);
        sLine.append(line.m_Instruction);
        sLine.append(" ");
        if (byteTokens.empty()) {
          sText.append(sLine);
        }
        byteTokens.add(line.m_Operand, line.m_Referenced, sLine.c_str());
      } else {
        byteTokens.flush();
        sLine.left_justify(tab_size);
        sLine.append(line.m_Instruction);
        sLine.append(" ");
        sLine.append(line.m_Operand);
        if (!line.m_Referenced && m_CommentUnusedCode) {
          sLine.left_justify(70 + tab_size + 6).append("; Unreferenced code");
        }
        sText.append(sLine);
        sText.append('\n');
      }
    }
  }
  { // Check if we have undefined labels
    for (auto it = m_Labels.begin(); it != m_Labels.end(); ++it) {
      //if (it->second->m_Name == "la289") {
      //  int debug = 0;
      //}
      if (it->second->m_Address < 0x0100) {
        continue;
      } else if (!it->second->m_IsGenerated && !it->second->m_IsAlias) {
        auto lbl = it->second;
        auto alias = getLabel(lbl->m_Address - 1);
        if (alias && !alias->m_IsAlias && alias->m_IsGenerated) {
          Hue::Util::String sAlias(alias->m_Name); 
          sAlias.append("+1");
          sText.replace(lbl->m_Name, sAlias);
        } else if ((alias = getLabel(lbl->m_Address - 2)) != NULL) {
          assert(!alias->m_IsAlias);
          assert(alias->m_IsGenerated);
          Hue::Util::String sAlias(alias->m_Name); 
          sAlias.append("+2");
          sText.replace(lbl->m_Name, sAlias);
        } else {
          // Look for nearest defined label
          auto sAlias = formatAddressUsingClosestLabel(lbl->m_Address, false);
          sText.replace(lbl->m_Name, sAlias);
        }
      }
    }
  }
  return sText;
}

} // namespace SASM
