
#include "Label.h"

namespace SASM {

Label* Label::create(Token const& nameToken, int64_t offset, InputFileID file_id, TextSpan const& span, int sectionID) {
  Label* label = (Label*)malloc(sizeof(Label) + nameToken.length());
  label->m_FileID     = file_id;
  label->m_Span       = span;
  label->m_Offset     = offset;
  label->m_SectionID  = sectionID;
  char* write = label->m_Name;
  const char* read = nameToken.m_pzTokenStart;
  while (read < nameToken.m_pzTokenEnd) {
    *write++ = *read++;
  }
  *write = '\0';
  //sasm_printf("Label:'%s'\n", label->m_Name);
  return label;
}

Label* Label::create(const char* name, int64_t offset, InputFileID file_id, TextSpan const& span, int sectionID) {
  Token nametoken(name);
  return create(nametoken, offset, file_id, span, sectionID);
}

void Label::destroy(Label* label) {
  assert(label);
  free(label);
}

}
