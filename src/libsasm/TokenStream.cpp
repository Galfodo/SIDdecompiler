
#include "TokenStream.h"

#include <assert.h>

namespace SASM {

TokenStream::TokenStream(GetToken_pf pfGetToken, char const* pzData) : m_pfGetToken(pfGetToken), m_pzData(pzData), m_CachedToken(false) {
  assert(m_pfGetToken != NULL);
}

bool TokenStream::next(Token& token) {
  if (m_CachedToken) {
    token = m_NextToken;
    m_CachedToken = false;
    return true;
  }
  return m_pfGetToken(token, &m_pzData, false);
}

bool TokenStream::peek(Token& token) {
  if (m_CachedToken) {
    token = m_NextToken;
    return true;
  } else if (m_pfGetToken(m_NextToken, &m_pzData, false)) {
    m_CachedToken = true;
    token = m_NextToken;
    return true;
  }
  return false;
}

} // namespace SASM
