
#include "UnresolvedReference.h"

namespace SASM {

UnresolvedReference::UnresolvedReference(int sectionID, int64_t pc, int64_t offset, AddrMode addrmode, const char* filename, int line) :
  m_SectionID(sectionID),
  m_PC(pc),
  m_Offset(offset),
  m_AddrMode(addrmode),
  m_FileName(filename),
  m_Line(line) 
{
}

}
