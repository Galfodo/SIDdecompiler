
#include "TextBuffer.h"

#include <assert.h>

namespace SASM {

TextBuffer::TextBuffer() : m_Buffer(NULL) {
}

TextBuffer::~TextBuffer() {
  clear();
}

void TextBuffer::clear() {
  if (m_Buffer) {
    m_Buffer->release();
    m_Buffer = NULL;
  }
}

bool TextBuffer::load(const char* pzFileName) {
  clear();
  m_Buffer = RefCountedBuffer::load(pzFileName);
  return m_Buffer != NULL;
}

TextBuffer::TextBuffer(TextBuffer const& rhs) : m_Buffer(NULL) {
  *this = rhs;
}

TextBuffer::TextBuffer(const char* contents) : m_Buffer(NULL) {
  *this = contents;
}

TextBuffer::TextBuffer(Hue::Util::String const& contents) : m_Buffer(NULL) {
  *this = contents;
}

Hue::Util::String const& TextBuffer::contents() const {
  if (m_Buffer) {
    return m_Buffer->contents();
  } else {
    static Hue::Util::String sEmpty;
    return sEmpty;
  }
}

const char* TextBuffer::c_str() const {
  return contents().c_str();
}

TextBuffer& TextBuffer::operator= (TextBuffer const& rhs) {
  clear();
  m_Buffer = rhs.m_Buffer;
  if (m_Buffer) {
    m_Buffer->addRef();
  }
  return *this;
}

TextBuffer& TextBuffer::operator= (Hue::Util::String const& contents) {
  clear();
  m_Buffer = new RefCountedBuffer(contents.c_str());
  return *this;
}

TextBuffer& TextBuffer::operator= (const char* pzContents) {
  clear();
  m_Buffer = new RefCountedBuffer(pzContents);
  return *this;
}

TextBuffer::RefCountedBuffer::RefCountedBuffer() : m_RefCount(1) {
}

TextBuffer::RefCountedBuffer::RefCountedBuffer(const char* contents) : m_RefCount(1) {
  m_Buffer.assign(contents);
}

int TextBuffer::RefCountedBuffer::addRef() {
  ++m_RefCount;
  return m_RefCount;
}

int TextBuffer::RefCountedBuffer::release() {
  assert(m_RefCount > 0);
  --m_RefCount;
  if (m_RefCount == 0) {
    delete this;
    return 0;
  } else {
    return m_RefCount;
  }
}

TextBuffer::RefCountedBuffer* TextBuffer::RefCountedBuffer::load(const char* pzFileName) {
  RefCountedBuffer* buffer = new RefCountedBuffer();
  if (buffer->m_Buffer.load(pzFileName)) {
    return buffer;
  } else {
    delete buffer;
    return NULL;
  }
}

}
