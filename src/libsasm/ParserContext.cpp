
#include "ParserContext.h"

namespace SASM {

ParserContext::ParserContext() : m_Pos(0), m_EOF(false) {
}

ParserContext::ParserContext(ParserContext const& rhs) {
  *this = rhs;
}

ParserContext::~ParserContext() {
  clear();
}

void ParserContext::clear() {
}

ParserContext& ParserContext::operator=(ParserContext const& rhs) {
  clear();
  m_Pos = rhs.m_Pos;
  m_EOF = rhs.m_EOF;
  m_Name = rhs.m_Name;
  m_Tokens = rhs.m_Tokens;
  m_TextBuffer = rhs.m_TextBuffer;
  return *this;
}

ParserContext& ParserContext::setPos(int pos) {
  assert(pos >= 0);
  if (pos >= (int)m_Tokens.size()) {
    m_Pos = (int)m_Tokens.size();
    m_EOF = true;
  } else {
    m_Pos = pos;
    m_EOF = false;
  }
  return *this;
}

Token const& ParserContext::current() const {
  return peek(0);
}

Token const& ParserContext::prev() const {
  return peek(-1);
}

Token const& ParserContext::next() const {
  return peek(1);
}

Token const& ParserContext::peek(int offset_from_current) const {
  return at(m_Pos + offset_from_current);
}

Token const& ParserContext::at(int pos) const {
  if (pos < 0 || pos >= (int)m_Tokens.size()) {
    static Token empty;
    return empty;
  }
  return m_Tokens[pos];
}

bool ParserContext::move(int offset_from_current) {
  int pos = m_Pos + offset_from_current;
  if (pos < 0) {
    m_Pos = 0;
    return false;
  }
  if (pos >= (int)m_Tokens.size()) {
    m_Pos = (int)m_Tokens.size();
    m_EOF = true;
    return false;
  }
  m_Pos = pos;
  return true;
}

ParserContext& ParserContext::rewind() {
  m_Pos = 0;
  m_EOF = false;
  return *this;
}

int ParserContext::tokenCount() {
  return (int)m_Tokens.size();
}

ParserContext& ParserContext::removeTokenAt(int index) {
  m_Tokens.erase(m_Tokens.begin() + index);
  return *this;
}

void ParserContext::pushstate(int pos) {
  m_StateStack.push(m_Pos);
  m_Pos = pos;
  m_EOF = (m_Pos >= (int)m_Tokens.size());
}

void ParserContext::popstate() {
  m_Pos = m_StateStack.top();
  m_EOF = (m_Pos >= (int)m_Tokens.size());
}

} // namespace SASM
