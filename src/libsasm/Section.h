#ifndef SASM_SECTION_H_INCLUDED
#define SASM_SECTION_H_INCLUDED

#include <vector>
#include <stdint.h>
#include <assert.h>

#include "Types.h"

namespace SASM {

class Section {
public:
  enum class Attributes {
    None = 0,
    BSS = 1,
    ROM = 2,
    RAM = None
  };
                      Section(const char* name, int sectionID, Attributes attr = Attributes::RAM);
  inline int64_t      currentOffset() const { return m_Data.size(); }
  inline int64_t      currentPC() const { assert(isPCvalid()); return this->m_StartPC + currentOffset(); }
  inline bool         isPCvalid() const { return this->m_StartPC >= 0; }
  bool                setPC(int64_t pc);
  void                setORG(int64_t org);

  std::vector<byte>   m_Data;
  int64_t             m_StartPC;
  int64_t             m_ORG;
  Hue::Util::String   m_Name;
  Attributes          m_Attr;
  int                 m_ID;
};

}

#endif
