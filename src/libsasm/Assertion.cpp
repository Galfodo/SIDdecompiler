
#include "Assertion.h"

namespace SASM {

Assertion::Assertion(int sectionID, int64_t pc, TokenList const& expressionTokens, const char* filename, int line) :
  m_SectionID(sectionID),
  m_PC(pc),
  m_ExpressionTokens(expressionTokens),
  m_FileName(filename),
  m_Line(line) 
{
}

}
