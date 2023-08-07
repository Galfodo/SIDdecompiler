
#include "UnresolvedReference.h"

namespace SASM {

UnresolvedReference::UnresolvedReference(int sectionID, int64_t pc, int64_t offset, AddrMode addrmode, InputFileID file_id, TextSpan const& span) :
  m_SectionID(sectionID),
  m_PC(pc),
  m_Offset(offset),
  m_AddrMode(addrmode),
  m_FileID(file_id),
  m_Span(span)
{
}

} // namespace SASM
