#ifndef SASM_PROFILING_H_INCLUDED
#define SASM_PROFILING_H_INCLUDED

#include "DebugEvaluators.h"
#include "TraceOperand.h"

#include <vector>

namespace SASM {

struct MemAccessMap {
  enum {
    ROW_SIZE                  = 64
  };
  enum AccessType {
    UNTOUCHED                 = 0,
    READ                      = (1 << 0),
    WRITE                     = (1 << 1),
    EXECUTE                   = (1 << 2),
    OPERAND                   = (1 << 3)
  };

                              MemAccessMap(int addr, int size);
                              ~MemAccessMap();
  int                         getAccessType(int address);
  Hue::Util::String           printMap(int firstaddr, int lastaddr);
  void                        dump();
  inline void                 recordAccess(int addr, int accesstype) {
                                if (addr >= m_Address && addr < m_Address + m_Size) {
                                  m_Data[addr - m_Address] |= accesstype;
                                  if (m_ExcludeIO && addr >= 0xd000 && m_ExcludeIO && addr < 0xe000) {
                                    return;
                                  }
                                  if (addr < m_LowestAccessed) {
                                    m_LowestAccessed = addr;
                                  }
                                  if (addr > m_HighestAccessed) {
                                    m_HighestAccessed = addr;
                                  }
                                }
                              }
  inline OperandTraceState&   traceState() { return m_TraceState; }       

  int                         m_Address;
  int                         m_Size;
  int                         m_LowestAccessed;
  int                         m_HighestAccessed;
  bool                        m_ExcludeIO;
  byte*                       m_Data;
  OperandTraceState           m_TraceState;
  std::vector<DestructibleBase> 
                              m_Managed;
};

struct Profiling {

static void                   attachMemAccessMap(OperandTracerEmu& emu, MemAccessMap* memAccessMap, int relocationRegionAddress, int relocationRegionSize);

};

} // namespace SASM

#endif
