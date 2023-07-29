#ifndef SASM_PARSERCONTEXT_H_INCLUDED
#define SASM_PARSERCONTEXT_H_INCLUDED

#include "TokenStream.h"
#include "TextBuffer.h"

#include <stack>

namespace SASM {

class ParserContext {
public:
                            ParserContext();
                            ParserContext(ParserContext const& rhs);
                            ~ParserContext();
  ParserContext&            operator=(ParserContext const& rhs);
  Token const&           current() const;
  Token const&           prev() const;
  Token const&           next() const;
  Token const&           peek(int offset_from_current) const;
  Token const&           at(int pos) const;
  bool                      move(int offset_from_current = 1);
  ParserContext&            rewind();
  int                       pos() const { return m_Pos; }
  ParserContext&            setPos(int pos);
  inline bool               eof() const { return m_EOF; }
  inline const char*        name() const { return m_Name.c_str(); }
  inline ParserContext&     setName(const char* pzName) { m_Name = pzName; return *this; }
  int                       tokenCount();
  ParserContext&            removeTokenAt(int index);
  inline void               pushstate() { pushstate(pos()); }
  void                      pushstate(int pos);
  void                      popstate();
  inline TextBuffer const&  textBuffer() const { return m_TextBuffer; }
private:
  void                      clear();
  TextBuffer                m_TextBuffer;
  Hue::Util::String         m_Name;
  std::vector<Token>     m_Tokens;
  int                       m_Pos;
  bool                      m_EOF;
  std::stack<int>           m_StateStack;
};

}

#endif
