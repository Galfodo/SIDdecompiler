
#include <assert.h>
#include <ctype.h>

#include "Tokenizer.h"
#include "TokenList.h"
#include "HueUtil/RegExp.h"

namespace SASM {

bool
Tokenizer::GetToken(Token& token, char const** ppzSource, bool include_space) {
  assert(ppzSource);
  assert(*ppzSource);

  unsigned char const*
    pzSource = (unsigned char*)*ppzSource;

  unsigned char const*
    pzStart = pzSource;

  bool
    isFloat = false;

  token.clear();
  while (*pzSource == ' ' || *pzSource == '\t') {
    ++pzSource;
  }
  if (include_space && pzStart != pzSource) {
    token.m_pzTokenStart = (char*)pzStart;
    token.m_pzTokenEnd = (char*)pzSource;
    token.m_Classification = Token::Whitespace;
    return true;
  }
  if (*pzSource)
  {
    token.m_pzTokenStart = (char*)pzSource;
    switch(*pzSource)
    {
    case '\r':
      if (pzSource[1] == '\n') {
        ++pzSource;
      } else {
        // just a return.. tread it as a newline nonetheless
      }
      // fall through
    case '\n':
      // Treat newline as a special token
      token.m_Classification = Token::NewLine;
      ++pzSource;
      break;

    case '>': case '<':
      // >>=
      // >>
      // >=
      // >
      // <<=
      // <<
      // <=
      // <
      if (pzSource[1] == pzSource[0]) {
        ++pzSource;
      }
      pzSource++;
      if (*pzSource == '=') ++pzSource;
      token.m_Classification = Token::Operator;
      break;
    case '&': case '|':
    case '+': case '-':
      ++pzSource;
      token.m_Classification = Token::Operator;
      break;
    case ':': case '=':
      // ==
      // ::
      if (pzSource[1] == pzSource[0]) {
        ++pzSource;
      }
      pzSource++;
      token.m_Classification = Token::Operator;
      break;
    case '%':
      if (pzSource[1] == '0' || pzSource[1] == '1') {
        ++pzSource;
        while (pzSource[0] == '0' || pzSource[0] == '1') {
          ++pzSource;
        }
        token.m_Classification = Token::BinaryNumber;
        break;
      }
      // else fall through...
    case '!': case '*': case '/':
      // !=
      // !
      // *=
      // *
      // // comment
      // /* comment */
      // /=
      // /
      // %=
      // %
      if (pzSource[0] == '/' && pzSource[1] == '/') {
        while (pzSource[0] != '\r' && pzSource[0] != '\n' && pzSource[0] != '\0') {
          ++pzSource;
        }
        token.m_Classification = Token::Comment;
      } else if (pzSource[0] == '/' && pzSource[1] == '*') {
        pzSource += 2;
        while (pzSource[0] != '\0' && !(pzSource[0] == '*' && pzSource[1] == '/')) {
          ++pzSource;
        }
        if (pzSource[0]) {
          pzSource += 2;
        }
        token.m_Classification = Token::Comment;
      } else {
        pzSource++;
        if (*pzSource == '=') ++pzSource;
        token.m_Classification = Token::Operator;
      }
      break;
    case ';':
      ++pzSource;
      while (pzSource[0] != '\r' && pzSource[0] != '\n' && pzSource[0] != '\0') {
        ++pzSource;
      }
      token.m_Classification = Token::Comment;
      break;
    case ',': case '#': case '(': case ')': case '[': case ']': case '{': case '}':
      pzSource++;
      token.m_Classification = Token::Operator;
      break;
    case '.':
      pzSource++;
      if (isdigit(pzSource[0])) {
        isFloat = true;
        goto parse_fraction;
      } else if (isalpha(pzSource[0])) {
        token.m_Classification = Token::Directive;
        goto parse_identifier;
      }
      break;
    case '$':
      ++pzSource;
      while (isxdigit(pzSource[0])) {
        ++pzSource;
      }
      token.m_Classification = Token::HexNumber;
      break;
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
      // Number
      if (pzSource[0] == '0' && (pzSource[1] == 'x' || pzSource[1] == 'X')) {
        pzSource += 2;
        while (isxdigit(pzSource[0])) {
          ++pzSource;
        }
        token.m_Classification = Token::HexNumber;
      } else {
        token.m_Classification = Token::Integer;
        while (isdigit(pzSource[0])) {
          ++pzSource;
        }
        if (pzSource[0] == '.') {
          ++pzSource;
          isFloat = true;
          token.m_Classification = Token::Real;
        }
parse_fraction:
        while (isdigit(pzSource[0])) {
          ++pzSource;
        }
        if (pzSource[0] == 'e' || pzSource[0] == 'E') {
          token.m_Classification = Token::Real;
          unsigned char const* temp = pzSource;
          ++temp;
          if (temp[0] == '+' || temp[0] == '-') {
            ++temp;
          }
          if (isdigit(temp[0])) {
            isFloat = true;
            pzSource = temp;
          }
          while (isdigit(pzSource[0])) {
            ++pzSource;
          }
        }
        while (pzSource[0] == 'u' || pzSource[0] == 'U' || pzSource[0] == 'l' || pzSource[0] == 'L') {
          ++pzSource;
        }
        if (isFloat && pzSource[0] == 'f' || pzSource[0] == 'F') {
          ++pzSource;
        }
      }
      break;
    case '\'':
      // Character literal
      pzSource++;
      if (*pzSource == '\0' || pzSource[1] != '\'')
      {
        return false;
      }
      if (pzSource[0] == '\\')
      {
        if (pzSource[1] != '0')
        {
          return false;
        }
        ++pzSource;
      }
      ++pzSource;
      if (*pzSource != '\'')
      {
        return false;
      }
      token.m_Classification = Token::Char;
      ++pzSource;
      break;

    case '\"':
      // Quoted string
      pzSource++;
      while (*pzSource != '\"')
      {
        if (*pzSource == '\0' || (pzSource[0] == '\\' && pzSource[1] == '\0'))
        {
          return false;
        }
        else if (*pzSource == '\\')
        {
          pzSource++;
        }
        pzSource++;
      }
      pzSource++;
      token.m_Classification = Token::String;
      break;
    default:
      token.m_Classification = Token::Identifier;
parse_identifier:
      // Identifier
      pzSource++;
      while (isalpha(*pzSource) || isdigit(*pzSource) || *pzSource == '_' || *pzSource == '@')
      {
        pzSource++;
      }
      break;
    }
    assert(token.m_Classification != Token::None);
    token.m_pzTokenEnd = (char*)pzSource;
    *ppzSource = (char*)pzSource;
    return true;
  }
  return false;
}

TokenList Tokenizer::Tokenize(const char* pzSource) {
  Token token;
  const char* pzWork = pzSource;
  TokenList tokens;
  while (GetToken(token, &pzWork, false)) {
    tokens.push_back(token);
  }
  return tokens;
}

} // namespace SASM
