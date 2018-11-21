#ifndef SASM_DISASSEMBLER_H_INCLUDED
#define SASM_DISASSEMBLER_H_INCLUDED

#include "Types.h"

#include <vector>
#include <map>
#include <set>

namespace SASM {

struct MemAccessMap;

class Disassembler {
public:
  bool                  m_AllowIllegals;
  bool                  m_UnknownAlwaysData;
  bool                  m_CommentUnusedData;
  bool                  m_CommentUnusedCode;
  bool                  m_RelocZP;
  int                   m_RelocZPaddr;
  bool                  m_IgnoreIOArea;

  enum RegionType {
    UNKNOWN = 0,
    CODE,
    DATA
  };

  struct Region {
                        Region(int addr, int size, RegionType type);
    int                 m_Address;
    int                 m_Size;
    RegionType          m_Type;
  };

  struct Label {
                        Label(int addr, const char* label, bool isAlias, bool executeableAddress);
    int                 m_Address;
    Hue::Util::String   m_Name;
    bool                m_IsAlias;
    bool                m_ExecutableAddressHint;
    bool                m_IsGenerated;
  };

  struct Line {
                        Line(int addr, Hue::Util::String const& instr, Hue::Util::String const& op, bool referenced);
    int                 m_Address;
    Hue::Util::String   m_Instruction;
    Hue::Util::String   m_Operand;
    bool                m_Referenced;
  };

  struct AddressTable {
                        AddressTable(const char* label_prefix, int loaddr, int hiaddr, int size);
                        AddressTable(const char* label_prefix, int wordaddr, int size);
    Hue::Util::String   formatLabel(Disassembler& disasm, int addr, int index, bool isLowByte);
    Hue::Util::String   formatLabel(Disassembler& disasm, int addr, int index);
    Hue::Util::String   m_LabelPrefix;
    int                 m_LoAddr;
    int                 m_HiAddr;
    int                 m_Size;
    int                 m_Distance; // Distance between elements. 1 for separate low/high tables, 2 for word tables, in which case m_HiAddr == m_LoAddr + 1
  };
                        Disassembler(byte* mem, int addr, int size, bool commentUnusedData, bool commentUnusedCode);
                        ~Disassembler();

  void                  parseMemAccessMap(MemAccessMap* map);
  void                  preprocessRegions();
  void                  disassemble();
  AddressTable*         addAddressTable(const char* label_prefix, int addrlo, int addrhi, int length);
  AddressTable*         addWordAddressTable(const char* label_prefix, int addr, int length);
  AddressTable*         addRelocAddress(const char* label, int addrlo, int addrhi);
  void                  addRegion(Region const& r);
  void                  addRegion(int addr, int size, RegionType type);
  Region const*         getRegion(int addr);
  Region const*         getNextRegion(int addr);
  Region const*         getPrevRegion(int addr);
  Label*                getOrCreateLabel(int addr, const char* name, bool executeableAddress);
  Label*                getLabel(int addr);
  Label*                addLabel(int addr, const char* name, bool executeableAddress);
  Label*                addOffsetAlias(Label* alias, int offset);
  Label*                addAlias(int addr, const char* alias);
  inline bool           isAddressInRange(int addr) const {
                          if (m_IgnoreIOArea && addr >= 0xd000 && addr < 0xe000) {
                            return false;
                          }
                          return addr >= m_Address && addr < m_Address + m_Size;
                        }
  Hue::Util::String     formatAddressUsingClosestLabel(int addr, bool force);
  Hue::Util::String     dump(int relocaddr);
  int                   getAddressFromTableAt(AddressTable* tbl, int index);
  void                  setHeader(const char* header);
  static Hue::Util::String     
                        defaultLabel(int addr);
private:
  bool                  addressInRelocTable(AddressTable** table, int* index, bool* isLowbyte, int addr);
  void                  formatAddress(Hue::Util::String& str, int addr, bool executeableAddress, bool forceLabel);
  Line*                 formatOpcode(byte* data, int address, bool referenced);
  byte                  getByteAt(int address);

  byte*                 m_Data;
  int                   m_Address;
  int                   m_Size;
  std::vector<Region*>  m_Regions;
  std::map<int, Label*> m_Labels;
  std::map<int, Line*>  m_Lines;
  std::vector<Label*>   m_Alias;
  std::vector<AddressTable*>
                        m_AddressTables;
  std::set<int>         m_ZPused;
  Hue::Util::String     m_Header;
};

} // namespace SASM

#endif
