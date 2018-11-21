
#include "STMemBuf.h"

#include <stdlib.h>
#include <stdio.h>

STMemBuf::STMemBuf() : m_Data(0), m_Size(0) {
}

STMemBuf::~STMemBuf() {
  resize(0);
}

void STMemBuf::resize(size_type size) {
  if (size == 0) {
    free(m_Data);
    m_Data = 0;
  } else if (size > m_Size) {
    m_Data = (unsigned char*)realloc(m_Data, size);
  }
  m_Size = size;
}

int STMemBuf::load(const char* filename) {
  FILE* f = fopen(filename, "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    size_type size = (size_type)ftell(f);
    resize(size);
    fseek(f, 0, SEEK_SET);
    fread(m_Data, size, 1, f);
    fclose(f);
    return STMemBuf::OK;
  }
  return STMemBuf::Error;
}

