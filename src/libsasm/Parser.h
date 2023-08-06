#ifndef SASM_PARSER_H_INCLUDED
#define SASM_PARSER_H_INCLUDED

#include "ParserContext.h"

namespace SASM {

class TextBuffer;

class Parser {
public:
          Parser();
  virtual ~Parser();
  bool    addFile(const char* pzFileName);
  bool    addFiles(Hue::Util::String::List const& filenames);

  void    reportError(Hue::Util::String const& message, const char* file, int line);

  std::vector<ParserContext> m_ParserContexts;
  std::vector<TextBuffer*> m_TextBuffers;

  int m_PC;
  int m_ORG;
};

} // namespace SASM

#endif
