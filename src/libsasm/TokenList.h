#ifndef SASM_TOKENLIST_H_INCLUDED
#define SASM_TOKENLIST_H_INCLUDED

#include "Token.h"
#include <vector>

namespace SASM {

class TokenList {
public:
                      TokenList();
                      TokenList(TokenList const& rhs);
  TokenList           rest(int start_index) const;
  inline void         consume(int count) { m_ReadPos += count; }
  bool                match_at(TokenList const& tokens, int pos) const;
  bool                match_start(const char* tokenstring) const;
  bool                match_end(const char* tokenstring) const;
  //bool                match_all(const char* token) const;
  bool                match_all(char token) const;
  bool                match_start(TokenList const& tokens) const;
  bool                match_end(TokenList const& tokens) const;
  Hue::Util::String   toString() const;
  Hue::Util::String   join(const char* separator) const;
  inline Token const& at(int pos) const { return m_Tokens.at(pos + m_ReadPos); }
  inline Token&       at(int pos) { return m_Tokens.at(pos + m_ReadPos); }
  inline bool         empty() const { return m_ReadPos >= (int)m_Tokens.size(); }
  inline int          size() const { int siz = (int)m_Tokens.size() - m_ReadPos; return siz > 0 ? siz : 0; }
  inline void         push_back(Token const& token) { m_Tokens.push_back(token); }
  void                clear();
private:
  int                 m_ReadPos;
  std::vector<Token>  m_Tokens;
};

}

#endif
