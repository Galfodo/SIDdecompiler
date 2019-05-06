#ifndef SASM_TYPES_H_INCLUDED
#define SASM_TYPES_H_INCLUDED

#define HUE_USE_FAST_STRING_SEARCH 0
#include "HueUtilString.h"

#ifndef _MSC_VER
#define override 
#endif

#include <stdint.h>

namespace SASM {

typedef unsigned char byte;
typedef unsigned short word;

struct DestructibleBase {
  virtual ~DestructibleBase() {};
  virtual void Destroy() = 0;
};

typedef int InputFileID;

struct TextSpan {
  int m_Line;
  int m_ColumnBegin;
  int m_ColumnEnd;

  TextSpan(int line, int column_begin = 0, int column_end = -1) :
    m_Line(line),
    m_ColumnBegin(column_begin),
    m_ColumnEnd(column_end) {
  }
};

}

#endif
