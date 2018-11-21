
#include "TokenList.h"
#include "Tokenizer.h"

namespace SASM {

TokenList::TokenList() : m_ReadPos(0) {
  m_Tokens.reserve(32);
}

TokenList::TokenList(TokenList const& rhs) : m_ReadPos(rhs.m_ReadPos), m_Tokens(rhs.m_Tokens) {
}

void TokenList::clear() {
  m_Tokens.clear();
  m_ReadPos = 0;
}

TokenList TokenList::rest(int start_index) const {
  TokenList result;
  int newsize = this->size() - start_index;
  if (newsize > 0) {
    result.m_Tokens.resize(newsize);
    Token const* read = &this->at(start_index);
    Token* write = &result.at(0);
    while (newsize) {
      *write++ = *read++;
      --newsize;
    }
  }
  return result;
}

bool TokenList::match_at(TokenList const& tokens, int pos) const {
  if (this->size() < tokens.size()) {
    return false;
  }
  if (pos < 0) {
    return false;
  }
  for (int i = 0; i < tokens.size(); ++i) {
    if (!this->at(pos + i).fmatch(tokens.at(i))) {
      return false;
    }
  }
  return true;
}

bool TokenList::match_start(const char* tokenstring) const {
  TokenList tokens = Tokenizer::Tokenize(tokenstring);
  return match_start(tokens);
}

bool TokenList::match_end(const char* tokenstring) const {
  TokenList tokens = Tokenizer::Tokenize(tokenstring);
  return match_end(tokens);
}

bool TokenList::match_start(TokenList const& tokens) const {
  return match_at(tokens, 0);
}

bool TokenList::match_end(TokenList const& tokens) const {
  return match_at(tokens, this->size() - tokens.size());
}

Hue::Util::String TokenList::toString() const {
  return join("");
}

Hue::Util::String TokenList::join(const char* separator) const {
  Hue::Util::String result;
  for (int i = 0; i < this->size(); ++i) {
    if (!result.empty()) {
      result.append(separator);
    }
    result.append(this->at(i).toString());
  }
  return result;
}

bool TokenList::match_all(char token) const {
  if (this->empty()) {
    return false;
  } else {
    for (int i = 0; i < this->size(); ++i) {
      if (!this->at(i).equals(token)) {
        return false;
      }
    }
    return true;
  }
}

}
