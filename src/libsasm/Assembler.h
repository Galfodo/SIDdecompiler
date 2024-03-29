#ifndef SASM_ASSEMBLER_H_INCLUDED
#define SASM_ASSEMBLER_H_INCLUDED

#include <stdint.h>
#include <limits.h>
#include <vector>
#include <map>
#include <stack>
#include <ctime>

#include "Section.h"
#include "AddrMode.h"
#include "Assertion.h"

#define SASM_ENABLE_DEBUGINFO 1

namespace SASM {

class Label;
class UnresolvedReference;
class Token;
class TokenList;
class Assertion;

class Assembler {
public:
  struct InputFileInfo {
    Hue::Util::String m_Filename;
#if SASM_ENABLE_DEBUGINFO
    char const*       m_Begin;
    char const*       m_End;
    std::vector<const char*>
                      m_Lines;
#endif
  };

  static const char*                                version();
                                                    Assembler();
                                                    ~Assembler();

  std::vector<byte>                                 assemble_file(const char* filename);
  std::vector<byte>                                 assemble_string(const char* source);
  std::vector<byte>                                 assemble(const char* source, const char* filename);
  int                                               errorcount();
  void                                              fprintErrors(FILE* file, int max_errors = INT_MAX);
  bool                                              write(const char* filename, std::vector<byte> const& data);
  inline std::vector<Assertion*> const&             assertions() const { return m_Assertions; }
//private:
  void                                              addLabel(const char* name, int64_t pc);
  void                                              addLabel(Token const& nametoken, int64_t pc);
  void                                              addUnresolvedLabel(const char* name, int64_t pc, TokenList& expressiontokens);
  void                                              addUnresolved(const char* expression, int64_t pc, int64_t offset, AddrMode addrMode);
  void                                              addUnresolved(TokenList& expressiontokens, int64_t pc, int64_t offset, AddrMode addrMode);
  bool                                              isAnonymousLabelReference(Hue::Util::String& labelname, TokenList& tokens);
  Label*                                            findLabel(const char* name);
  int                                               getAnonLabelIndex(int offset, char suffix);
  const char*                                       getAnonymousLabelName(Hue::Util::String& labelname, int offset, char suffix);
  const char*                                       getAnonymousLabel(Hue::Util::String& labelname, int offset, char suffix);
  void                                              addAnonymousLabel(int64_t value, char suffix);
  void                                              resolveAll();
  void                                              recordError(Hue::Util::String const& msg, InputFileID file_id, TextSpan const& span, bool record = true);
  inline Section&                                   currentSection() { return *m_CurrentSection; }
  inline Section const&                             currentSection() const { return *m_CurrentSection; }
  Section*                                          getOrCreateSection(const char* name, Section::Attributes attr);
  void                                              init();
  bool                                              parseConditionals(TokenList& tokens);
  void                                              parseConditionTrue(TokenList& tokens);
  void                                              parseConditionFalse(TokenList& tokens);
  void                                              directiveIf(bool condition);
  void                                              directiveElse();
  void                                              directiveEndif();
  int64_t                                           evaluateExpression(TokenList& tokens, int64_t pc, bool report_errors, bool consume_tokens = false);
  int64_t                                           evaluateExpressionImpl(bool& parse_error, TokenList& tokens, int64_t pc, bool report_errors, bool consume_tokens);
  inline int64_t                                    writeOffset() const { return currentSection().m_Data.size(); }
  void                                              writeByte(byte value);
  void                                              writeWord(word value);
  void                                              writeByteAt(byte value, int64_t offset);
  void                                              writeWordAt(word value, int64_t offset);
  byte                                              calculateBranch(int64_t source, int64_t target, InputFileID file_id, TextSpan const& span);
  void                                              registerDebugInfoRange(int64_t pc, InputFileID file_id, TextSpan const& span);
  void                                              writeInstruction(int opcode, InputFileID file_id, TextSpan const& span);
  void                                              writeInstructionOperand(int64_t pc, int64_t offset, int64_t operand, AddrMode addrMode, InputFileID file_id, TextSpan const& span);
  void                                              writePetsciiStringToMemory(Token const& token, bool isAscii);
  InputFileID                                       addFileInfo(const char* filename, const char* contents);
  bool                                              resolveTokenOrigin(int& inputfileID, int& linenumber, int& column_begin, int& column_end, Token const& token);
  InputFileInfo&                                    getInputFileInfo(int inputFileID);
  const char*                                       getInputFileName(int inputFileID);

  std::vector<InputFileInfo>                        m_InputFiles;
  Section*                                          m_CurrentSection;
  std::vector<Section*>                             m_Sections;
  std::map<Hue::Util::String, Label*>               m_Labels;
  std::vector<UnresolvedReference*>                 m_Unresolved;
  std::stack<bool>                                  m_ConditionStack;
  std::vector<Hue::Util::String>                    m_Errors;
  std::vector<Assertion*>                           m_Assertions;
  bool                                              m_EndOfSource;
  bool                                              m_AllowTASSLabels;
  int                                               m_AnonLabelIndexFwd;
  int                                               m_AnonLabelIndexBwd;
  int                                               m_LineNumber;
  InputFileID                                       m_CurrentFileID;
  int64_t                                           m_PC;
  int64_t                                           m_ORG;
  Hue::Util::String                                 m_Source;
  time_t                                            m_AssemblyTime;
  bool                                              m_CreateDebugInfo;
};

} // namespace SASM

#endif
