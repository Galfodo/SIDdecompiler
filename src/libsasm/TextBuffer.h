#ifndef SASM_TEXTBUFFER_H_INCLUDED
#define SASM_TEXTBUFFER_H_INCLUDED

#include "Types.h"

namespace SASM {

class TextBuffer {
public:
  TextBuffer();
  TextBuffer(TextBuffer const& rhs);
  TextBuffer(const char* contents);
  TextBuffer(Hue::Util::String const& contents);
  ~TextBuffer();

  bool load(const char* pzFileName);

  Hue::Util::String const& contents() const;
  const char* c_str() const;

  TextBuffer& operator= (TextBuffer const& rhs);
  TextBuffer& operator= (Hue::Util::String const& contents);
  TextBuffer& operator= (const char* pzContents);
private:
  void clear();

  class RefCountedBuffer {
  public:
    RefCountedBuffer();
    RefCountedBuffer(const char* contents);
    static RefCountedBuffer* load(const char* pzFileName);
    int addRef();
    int release();
    inline Hue::Util::String const& contents() const { return m_Buffer; }
  private:
    int m_RefCount;
    Hue::Util::String m_Buffer;
  };

  RefCountedBuffer* m_Buffer;
};

} // namespace SASM

#endif
