
#include "Section.h"

namespace SASM {

Section::Section(const char* name, int sectionID, Attributes attr) :
  m_Name(name),
  m_Attr(attr),
  m_StartPC(-1),
  m_ORG(0),
  m_ID(sectionID)
{
}

bool Section::setPC(int64_t pc) {
  if (isPCvalid()) {
    if (pc < currentPC()) {
      return false;
    }
    int64_t delta = pc - currentPC();
    assert(delta >= 0);
    if (delta > 0) {
      m_Data.resize(m_Data.size() + (size_t)delta);
    }
  } else {
    m_StartPC = pc;
  }
  return true;
}

void Section::setORG(int64_t org) {
  m_ORG = org;
}

} // namespace SASM
