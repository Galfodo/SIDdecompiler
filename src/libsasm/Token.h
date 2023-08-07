#ifndef SASM_PARSERTOKEN_H_INCLUDED
#define SASM_PARSERTOKEN_H_INCLUDED

#include <stdint.h>
#include "Types.h"

namespace SASM {

class Token {
public:
  enum {
    None = 0,
    Integer,
    Real,
    HexNumber,
    BinaryNumber,
    Identifier,
    Directive,
    Operator,
    Comment,
    String,
    Char,
    Whitespace,
    NewLine,
    ILLEGAL
  };

  char const        *m_pzTokenStart,
                    *m_pzTokenEnd;
  int               m_Classification;
                    Token(const char* value, int classification = None);
  inline            Token() : m_Classification(0), m_pzTokenEnd(NULL), m_pzTokenStart(NULL) { }
  inline int const& classification() const { return m_Classification; }
  inline void       clear() { m_pzTokenStart = m_pzTokenEnd = NULL; m_Classification = 0; }
  inline bool       empty() const { return m_pzTokenStart == m_pzTokenEnd; }
  bool              equals(const char* value) const;
  bool              equals(Hue::Util::String const& value) const;
  inline bool       equals(char value) const { return (length() == 1 && *m_pzTokenStart == value); }
  bool              equals(Token const& rhs) const;
  bool              starts_with(const char* prefix) const;
  bool              ends_with(const char* suffix) const;
  inline bool       isspace() const { return m_Classification == Whitespace || m_Classification == NewLine; }
  inline bool       isnumber() const { return m_Classification == Char || m_Classification == Integer || m_Classification == Real || m_Classification == HexNumber || m_Classification == BinaryNumber; }
  inline int        length() const { return (int)(m_pzTokenEnd - m_pzTokenStart); }
  inline char       at(int i) const { return m_pzTokenStart[i]; }
  bool              fmatch(Token const& rhs) const;
  Hue::Util::String toString() const;
  int64_t           integerValue() const;
  Token             unquoted() const;
};

} // namespace SASM

#endif
