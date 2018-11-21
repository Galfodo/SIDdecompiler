#ifndef SASM_LABEL_H_INCLUDED
#define SASM_LABEL_H_INCLUDED

#include "Token.h"
#include <stdlib.h>

namespace SASM {

class Label {
public:
  int64_t       m_Offset;
  const char*   m_FileName;
  int           m_Line;
  int           m_SectionID;
  char          m_Name[1];

  static Label* create(Token const& nameToken, int64_t offset, const char* filename, int line, int sectionID);
  static Label* create(const char* name, int64_t offset, const char* filename, int line, int sectionID);
  static void   destroy(Label* label);

private:
  ~Label();
};

}

#endif
