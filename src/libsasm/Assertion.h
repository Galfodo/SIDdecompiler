#ifndef SASM_ASSERTION_H_INCLUDED
#define SASM_ASSERTION_H_INCLUDED

#include "TokenList.h"

namespace SASM {

class Assertion {
public:
  int64_t     m_PC;
  int         m_SectionID;
  TokenList   m_ExpressionTokens;
  InputFileID m_FileID;
  TextSpan    m_Span;

              Assertion(int sectionID, int64_t pc, TokenList const& expressionTokens, InputFileID file_id, TextSpan const& span);
};

} // namespace SASM

#endif
