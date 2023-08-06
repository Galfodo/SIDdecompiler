#ifndef ASMTOKENIZER_H_INCLUDED
#define ASMTOKENIZER_H_INCLUDED

#include "Token.h"

namespace SASM {

class TokenList;

class Tokenizer {
public:
  static bool       GetToken(Token& token, char const** pSource, bool include_space);
  static TokenList  Tokenize(const char* source);
};

} // namespace SASM

#endif
