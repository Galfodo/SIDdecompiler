#ifndef SASM_ASSERTION_H_INCLUDED
#define SASM_ASSERTION_H_INCLUDED

#include "TokenList.h"

namespace SASM {

class Assertion {
public:
  int64_t     m_PC;
  int         m_SectionID;
  TokenList   m_ExpressionTokens;
  const char* m_FileName;
  int         m_Line;

              Assertion(int sectionID, int64_t pc, TokenList const& expressionTokens, const char* filename, int line);
};

}

#endif
