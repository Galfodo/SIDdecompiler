
#include "Label.h"

namespace SASM {

Label* Label::create(Token const& nameToken, int64_t offset, const char* filename, int line, int sectionID) {
  Label* label = (Label*)malloc(sizeof(Label) + nameToken.length());
  label->m_FileName   = filename;
  label->m_Line       = line;
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

Label* Label::create(const char* name, int64_t offset, const char* filename, int line, int sectionID) {
  Token nametoken(name);
  return create(nametoken, offset, filename, line, sectionID);
}

void Label::destroy(Label* label) {
  assert(label);
  free(label);
}

}
