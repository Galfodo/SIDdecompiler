
#include "Token.h"
#include <assert.h>
#include <set>

#ifdef _MSC_VER
#define strdup _strdup
#endif

struct ConstCharStarComparator
{
  bool operator()(const char *s1, const char *s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

typedef std::set<const char *, ConstCharStarComparator> stringset_t;


namespace SASM {

Token::Token(const char* value, int classification) {
  static stringset_t s_TokenTable;
  auto it = s_TokenTable.find(value);
  if (it == s_TokenTable.end()) {
    value = strdup(value);
    s_TokenTable.insert(value);
  } else {
    value = *it;
  }
  m_pzTokenStart = value;
  m_pzTokenEnd = m_pzTokenStart + strlen(value);
  m_Classification = classification;
}

Hue::Util::String Token::toString() const {
  Hue::Util::String result(m_pzTokenStart, m_pzTokenEnd);
  return result;
}

bool Token::equals(const char* pzValue) const {
  size_t len = strlen(pzValue);
  size_t thislen = (size_t)(m_pzTokenEnd - m_pzTokenStart);
  if (len == thislen) {
    if (strncmp(pzValue, m_pzTokenStart, len) == 0) {
      return true;
    }
  }
  return false;
}

bool Token::equals(Hue::Util::String const& value) const {
  if (value.length() == length()) {
    if (strncmp(value.c_str(), m_pzTokenStart, length()) == 0) {
      return true;
    }
  }
  return false;
}

bool Token::equals(Token const& rhs) const {
  if (length() == rhs.length()) {
    return strncmp(m_pzTokenStart, rhs.m_pzTokenStart, length()) == 0;
  }
  return false;
}

// Not entirely correct. Only works with 'sane' strings/identifiers
bool Token::fmatch(Token const& rhs) const {
  int count = length();
  if (count == rhs.length()) {
    const unsigned char* read1 = (const unsigned char*)this->m_pzTokenStart;
    const unsigned char* read2 = (const unsigned char*)rhs.m_pzTokenStart;
    while (count) {
      int c1 = *read1++;
      int c2 = *read2++;
      int r = c1 ^ c2;
      if (r != 0 && (r ^ 0x20) != 0) {
        return false;
      }
      --count;
    }
    return true;
  }
  return false;
}
 
bool Token::starts_with(const char* prefix) const {
  int prefixlength = (int)strlen(prefix);
  if (length() >= prefixlength) {
    if (strncmp(m_pzTokenStart, prefix, prefixlength) == 0) {
      return true;
    }
  }
  return false;
}

bool Token::ends_with(const char* suffix) const {
  int suffixlength = (int)strlen(suffix);
  if (length() >= suffixlength) {
    if (strncmp(m_pzTokenEnd - suffixlength, suffix, suffixlength) == 0) {
      return true;
    }
  }
  return false;
}


int64_t Token::integerValue() const {
  int64_t val = 0;
  switch (m_Classification) {
  case Char:
    assert(m_pzTokenEnd - m_pzTokenStart >= 3);
    if (m_pzTokenStart[1] == '\\')
    {
      assert(m_pzTokenStart[2] == '0');
      val = 0;
    } 
    else
    {
      val = m_pzTokenStart[1];
    }
    break;
  case Integer:
    assert(isdigit(*m_pzTokenStart));
    for (const char* work = m_pzTokenStart; work != m_pzTokenEnd; ++work) {
      assert(isdigit(*work));
      val = val * 10 + *work - '0';
    }
    break;
  case Real:
    assert(isdigit(*m_pzTokenStart));
    for (const char* work = m_pzTokenStart; work != m_pzTokenEnd; ++work) {
      if (!isdigit(*work)) {
        break;
      }
      val = val * 10 + *work - '0';
    }
    break;
  case HexNumber:
    {
      assert(m_pzTokenStart[0] == '$' || (m_pzTokenStart[0] == '0' && m_pzTokenStart[1] == 'x'));
      const char* work = m_pzTokenStart + 1;
      if (*work == 'x') {
        ++work;
      }
      assert(work <= m_pzTokenEnd);
      for (;work < m_pzTokenEnd; ++work) {
        assert(isxdigit(*work));
        char digit = *work;
        if (digit <= '9') {
          digit = digit - '0';
        } else {
          digit = tolower(digit) - 'a' + 10;
        }
        val = val * 16 + digit;
      }
    }
    break;
  case BinaryNumber:
    assert(*m_pzTokenStart == '%');
    for (const char* work = m_pzTokenStart + 1; work != m_pzTokenEnd; ++work) {
      assert(*work == '0' || *work == '1');
      val = val * 2 + *work - '0';
    }
    break;
  default:
    break;
  }
  return val;
}

Token Token::unquoted() const {
  if (*m_pzTokenStart == '"') {
    Token newtoken = *this;
    ++newtoken.m_pzTokenStart;
    --newtoken.m_pzTokenEnd;
    return newtoken;
  } else {
    return *this;
  }
}

} // namespace SASM
