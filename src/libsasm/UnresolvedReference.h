#ifndef SASM_UNRESOLVEDREFERENCE_H_INCLUDED
#define SASM_UNRESOLVEDREFERENCE_H_INCLUDED

#include "TokenList.h"
#include "AddrMode.h"

namespace SASM {

class UnresolvedReference {
public:
  int64_t           m_PC;
  int64_t           m_Offset;
  int               m_SectionID;
  AddrMode          m_AddrMode;
  TokenList         m_ExpressionTokens;
  const char*       m_FileName;
  int               m_Line;
  Hue::Util::String m_Label;

              UnresolvedReference(int sectionID, int64_t pc, int64_t offset, AddrMode addrmode, const char* filename, int line);
};

}
#endif
