
#include "Parser.h"
#include "Tokenizer.h"
#include "Output.h"

#include <ctype.h>
#include <vector>

namespace SASM {

Parser::Parser() :
  m_PC(0),
  m_ORG(0)
{
}

Parser::~Parser() {
  for (auto it = m_TextBuffers.begin(); it != m_TextBuffers.end(); ++it) {
    delete *it;
  }
  m_TextBuffers.clear();
}

struct LocalTokenStream {

  LocalTokenStream(std::vector<Token> const& tokens) : m_Tokens(tokens), m_Pos(0) {
  }

  inline bool empty() const { return m_Pos == m_Tokens.size(); }

  inline Token next() {
    if (!empty()) {
      return m_Tokens.at(m_Pos++);
    } else {
      static Token emptyToken;
      return emptyToken;
    }
  }

  size_t                     m_Pos;
  std::vector<Token> const&  m_Tokens;
};

bool Parser::addFile(const char* pzFileName) {

  TextBuffer
    *buf = new TextBuffer();

  m_TextBuffers.push_back(buf);
  if (buf->load(pzFileName)) {
    int
      linecount = 0;

    Token
      token;

    char const*
      pzParse = buf->c_str();

    TokenStream
      stream(&Tokenizer::GetToken, pzParse);

    if (*pzParse != '\0') {
      ++linecount;
    }

    std::vector<Token>
      currentTokens;

    while (stream.next(token)) {
      currentTokens.clear();
      while (token.classification() != Token::NewLine && token.classification() != Token::Comment && token.classification() != Token::None) {
        currentTokens.push_back(token);
        stream.next(token);
      }
      if (token.classification() == Token::NewLine) {
        ++linecount;
      }

      LocalTokenStream
        localTokens(currentTokens);

      if (!localTokens.empty()) {
        token = localTokens.next();
        switch (token.classification()) {
        case Token::Comment:
          break;
        case Token::Operator:
          if (token.equals(':')) {
          } else if (token.equals('*')) {
          } else {
            reportError(Hue::Util::String::static_printf("Syntax Error: '%s' can not appear here", token.toString().c_str()), pzFileName, linecount);
          }
          break;
        case Token::Identifier:
          break;
        case Token::Directive:
          {
            sasm_printf("%s\n", token.toString().c_str());
          }
          break;
        default:
          {
            reportError(Hue::Util::String::static_printf("Syntax Error"), pzFileName, linecount);
          }
        }
      }
    }
    //sasm_printf("%s linecount: %d\n", pzFileName, linecount);
    return true;
  } else {
    sasm_fprintf(stderr, "'%s' : file not found.\n", pzFileName);
  }
  return false;
}

bool Parser::addFiles(Hue::Util::String::List const& filenames) {
  bool all_ok = true;
  for (int i = 0; i < filenames.size(); ++i) {
    if (!addFile(filenames[i].c_str())) {
      all_ok = false;
    }
  }
  return all_ok;
}

void Parser::reportError(Hue::Util::String const& message, const char* filename, int line) {
  sasm_fprintf(stderr, "(%s,%d) error : %s\n", filename, line, message.c_str());
}

} // namespace SASM
