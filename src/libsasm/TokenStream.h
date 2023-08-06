#ifndef SASM_TOKENSTREAM_H_INCLUDED
#define SASM_TOKENSTREAM_H_INCLUDED

#include "Types.h"
#include "Token.h"

namespace SASM {

class TokenStream {
public:
  typedef bool (*GetToken_pf)(Token& token, char const** ppzSource, bool include_space);

  TokenStream(GetToken_pf pfGetToken, char const* pzData);
  bool next(Token& token);
  bool peek(Token& token);
private:
  GetToken_pf m_pfGetToken;
  Token m_NextToken;
  bool m_CachedToken;
  char const* m_pzData;
};

} // namespace SASM

#endif
