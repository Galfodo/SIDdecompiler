#ifndef SASM_LABEL_H_INCLUDED
#define SASM_LABEL_H_INCLUDED

#include "Token.h"
#include <stdlib.h>

namespace SASM {

class Label {
public:
  int64_t       m_Offset;
  InputFileID   m_FileID;
  TextSpan      m_Span;
  int           m_SectionID;
  char          m_Name[1];

  static Label* create(Token const& nameToken, int64_t offset, InputFileID file_id, TextSpan const& span, int sectionID);
  static Label* create(const char* name, int64_t offset, InputFileID file_id, TextSpan const& span, int sectionID);
  static void   destroy(Label* label);

private:
  ~Label();
};

} // namespace SASM

#endif
