
#include "Assertion.h"

namespace SASM {

Assertion::Assertion(int sectionID, int64_t pc, TokenList const& expressionTokens, InputFileID file_id, TextSpan const& span) :
  m_SectionID(sectionID),
  m_PC(pc),
  m_ExpressionTokens(expressionTokens),
  m_FileID(file_id),
  m_Span(span) 
{
}

}
