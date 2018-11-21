#ifndef ST_MEMBUF_H_INCLUDED
#define ST_MEMBUF_H_INCLUDED

#include <assert.h>

class STMemBuf {
public:
  typedef int size_type;
  typedef int index_type;

  enum {
    OK = 0,
    Error 
  };
  STMemBuf();
  ~STMemBuf();
  int load(const char* filename);
  void resize(size_type new_size);

  inline unsigned char* data() {
    return m_Data;
  }

  inline size_type size() const {
    return m_Size;
  }

  inline unsigned char& operator[](index_type index) {
    assert(index >= 0 && index < m_Size);
    return m_Data[index];
  }

private:
  unsigned char* m_Data;
  size_type m_Size;
};

#endif
