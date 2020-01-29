
#include "Assembler.h"
#include <stdio.h>
#include "TokenStream.h"
#include "Tokenizer.h"
#include "TokenList.h"
#include "Label.h"
#include "UnresolvedReference.h"
#include "OpcodeDefs.h"
#include "Output.h"
#include <chrono>
#include <stack>
#include <algorithm>

#define SASM_16BIT_TOKEN_OPERANDS_FORCES_ABSOLUTE_ADDRESSING 1 // Should an instruction operand specified as a 16-bit value force absolute addressing, even if it is less than 256? (e.g bit $00a9 -> 2c a9 00 (absolute addressing) instead of 24 a9 00 (zp addressing, default)

namespace SASM {

Assembler::Assembler() {
  this->init();
}

Section* Assembler::getOrCreateSection(const char* name, Section::Attributes attr) {
  for (size_t i = 0; i < m_Sections.size(); ++i) {
    if (m_Sections[i]->m_Name.equal(name)) {
      return m_Sections[i];
    }
  }
  Section* section = new Section(name, (int)m_Sections.size(), attr);
  m_Sections.push_back(section);
  return section;
}

Assembler::~Assembler() {
  // clean up...
}

void Assembler::init() {
  m_CurrentSection = getOrCreateSection("default", Section::Attributes::RAM);
  m_EndOfSource = false;
  m_AllowTASSLabels = true;
  m_LineNumber = 0;
  m_AnonLabelIndexFwd = 0;
  m_AnonLabelIndexBwd = 0;
  m_ConditionStack.push(true);
  m_Assertions.clear();
  m_Errors.clear();
  m_PC = 0;
  m_ORG = 0;
  m_AssemblyTime = 0;
  m_CreateDebugInfo = false;
}

std::vector<byte> Assembler::assemble_string(const char* str) {
  return assemble(str, "<stdin>");
}

std::vector<byte> Assembler::assemble_file(const char* filename) {
  Hue::Util::String 
    sContents;

  if (sContents.load(filename)) {
    return assemble(sContents.c_str(), filename);
  } else {
    m_AssemblyTime = 0;
    return std::vector<byte>();
  }
}

const char* Assembler::version() {
  return "SASM v0.31 (C) 2017-2020 Stein Pedersen/Prosonix";
}

std::vector<byte> Assembler::assemble(const char* source, const char* filename) {
  sasm_printf("Assembling file '%s'\n", filename);
  auto starttime = std::chrono::high_resolution_clock::now();
  m_AssemblyTime = 0;
  m_Source.assign(source);

  std::vector<byte> result;

  m_LineNumber = 0;

  m_CurrentFileID = addFileInfo(filename, source);

  Token
    token;

  char const*
    pzParse = m_Source.c_str();

  if (*pzParse) {
    ++m_LineNumber;
  }

  TokenStream
    stream(&Tokenizer::GetToken, pzParse);

  TokenList
    currentTokens;

  while (stream.next(token)) {
    currentTokens.clear();
    while (token.classification() != Token::NewLine && token.classification() != Token::Comment && token.classification() != Token::None) {
      currentTokens.push_back(token);
      stream.next(token);
    }
    if (!currentTokens.empty()) {
      if (m_ConditionStack.top()) {
        parseConditionTrue(currentTokens);
      } else {
        parseConditionFalse(currentTokens);
      }
    }
    if (token.classification() == Token::NewLine) {
      ++m_LineNumber;
    }
  }
  addLabel("__END__", m_PC);
  resolveAll();
  auto endtime = std::chrono::high_resolution_clock::now();
  auto deltatime = std::chrono::duration<double, std::milli>(endtime - starttime);
  sasm_printf("%s linecount: %d\n", getInputFileName(m_CurrentFileID), m_LineNumber);
  //sasm_printf("%d error(s) in %.03f milliseconds\n", (int)m_Errors.size(), deltatime);
  if (m_Errors.size() == 0) {
    result = currentSection().m_Data;
  }
  return result;
}

void Assembler::resolveAll()
{
  std::vector<UnresolvedReference*> unresolvedReferences = m_Unresolved;
  int prev_unresolved_count = -1;
  while (unresolvedReferences.size() > 0 && unresolvedReferences.size() != prev_unresolved_count)
  {
    std::vector<UnresolvedReference*> still_unresolved;
    for (int i = 0; i < (int)unresolvedReferences.size(); ++i)
    {
      UnresolvedReference* u = unresolvedReferences[i];
#ifdef _DEBUG
      Hue::Util::String sExpression =  u->m_ExpressionTokens.toString();
      //if (sExpression == "#ACTION_NORMAL")
      //{
      //  int debug = 0;
      //}
      //if (u->m_Label == "ACTION_NORMAL")
      //{
      //  int debug = 0;
      //}
#endif
      assert(u->m_SectionID == 0); // TODO!
      int64_t value = evaluateExpression(u->m_ExpressionTokens, u->m_Offset, false);
      if (value < 0)
      {
        still_unresolved.push_back(u);
      }
      else if (u->m_Label.length()) 
      {
        addLabel(u->m_Label.c_str(), value);
      } 
      else
      {
        writeInstructionOperand(u->m_PC, u->m_Offset, value, u->m_AddrMode, u->m_FileID, u->m_Span);
      }
    }
    prev_unresolved_count = (int)unresolvedReferences.size();
    unresolvedReferences = still_unresolved;
    still_unresolved.clear();
  }
  for (int i = 0; i < (int)unresolvedReferences.size(); ++i)
  {
    UnresolvedReference* u = unresolvedReferences[i];
    assert(u->m_SectionID == 0); // TODO!
    int64_t value = evaluateExpression(u->m_ExpressionTokens, u->m_Offset, true);
    if (value < 0)
    {
      recordError(Hue::Util::String::static_printf("Unresolved reference: '%s'", u->m_ExpressionTokens.toString().c_str()), u->m_FileID, u->m_Span, true);
    }
    else
    {
      writeInstructionOperand(u->m_PC, u->m_Offset, value, u->m_AddrMode, u->m_FileID, u->m_Span);
    }
  }
}

int Assembler::errorcount() {
  return (int)m_Errors.size();
}

bool Assembler::write(const char* filename, std::vector<byte> const& data) {
  size_t size = data.size();
  byte* bytes = NULL;
  if (size) {
    bytes = (byte*)&data[0];
  }
  FILE* f = fopen(filename, "wb");
  if (f) {
    if (size) {
      char
        header[2];

      header[0] = m_ORG & 0xff;
      header[1] = (m_ORG >> 8) & 0xff;
      fwrite(header, 2, 1, f);
      fwrite(bytes, size, 1, f);
    }
    fclose(f);
    return true;
  }
  return false;
}

int64_t Assembler::evaluateExpression(TokenList& tokens, int64_t pc, bool report_errors, bool consume_tokens) {
  bool error = false;
  int64_t value = evaluateExpressionImpl(error, tokens, pc, report_errors, consume_tokens);
  if (error)
  {
    return -1;
  }
  return value;
}

namespace TOK {

static const Token s_Ops[] = {
/*  LPAR */   ("("),
/*  RPAR */   (")"),
/*  NOT  */   ("!"),
/*  ASS  */   ("="),
/*  ADD  */   ("+"),
/*  SUB  */   ("-"),
/*  MUL  */   ("*"),
/*  DIV  */   ("/"),
/*  NEG  */   ("~"),
/*  XOR  */   ("^"),
/*  AND  */   ("&"),
/*  OR   */   ("|"),
/*  SHL  */   ("<<"),
/*  SHR  */   (">>"),
/*  LT   */   ("<"),
/*  GT   */   (">"),
/*  LE   */   ("<="),
/*  GE   */   (">="),
/*  EQU  */   ("=="),
/*  NEQ  */   ("!="),
/*  lAND */   ("&&"),
/*  lOR  */   ("||"),
/*  LO   */   ("LO"),
/*  HI   */   ("HI"),
/*  PC   */   ("PC")
};

}

namespace TOKID {

typedef enum {
  LPAR    , //("("),
  RPAR    , //(")"),
  NOT     , //("!"),
  ASS     , //("="),
  ADD     , //("+"),
  SUB     , //("-"),
  MUL     , //("*"),
  DIV     , //("/"),
  NEG     , //("~"),
  XOR     , //("^"),
  AND     , //("&"),
  OR      , //("|"),
  SHL     , //("<<"),
  SHR     , //(">>"),
  LT      , //("<"),
  GT      , //(">"),
  LE      , //("<="),
  GE      , //(">="),
  EQU     , //("=="),
  NEQ     , //("!="),
  lAND    , //("&&"),
  lOR     , //("||"),
  LO      , //("LO"),
  HI      , //("HI"),
  PC      , //("PC"),
  NONE
} ID; 

}

static TOKID::ID getop(Token& op) {
  for (int i = 0; i < TOKID::NONE; ++i) {
    if (op.fmatch(TOK::s_Ops[i])) {
      if (i == TOKID::ASS)
        i = TOKID::EQU;
      return (TOKID::ID)i;
    }
  }
  assert(0);
  return (TOKID::ID)0;
}

// Function to find precedence of  
// operators. 

static int precedence(TOKID::ID op){ 
  switch (op) {
  case TOKID::NOT: case TOKID::NEG:
    return 15 - 2;
  case TOKID::MUL: case TOKID::DIV: 
    return 15 - 3;
  case TOKID::ADD: case TOKID::SUB:
    return 15 - 4;
  case TOKID::SHL: case TOKID::SHR:
    return 15 - 5;
  case TOKID::LT: case TOKID::LE: case TOKID::GE: case TOKID::GT:
    return 15 - 6;
  case TOKID::EQU: case TOKID::NEQ:
    return 15 - 7;
  case TOKID::AND:
    return 15 - 8;
  case TOKID::XOR:
    return 15 - 9;
  case TOKID::OR:
    return 15 - 10;
  case TOKID::lAND:
    return 15 - 11;
  case TOKID::lOR:
    return 15 - 12;
  case TOKID::HI: case TOKID::LO:
    return 15 - 13;
  case TOKID::LPAR:
    return 15 - 15;
  default:
    assert(0);
    return 15;
  }
} 

static bool isunary(TOKID::ID op) {
  switch (op) {
  case TOKID::NOT: case TOKID::NEG: 
  case TOKID::LO: case TOKID::HI:
    return true;
  default:
    return false;
  }
}

static int64_t applyUnaryOp(TOKID::ID op, int64_t a) {
  switch (op) {
  case TOKID::NOT:
    return !a;
  case TOKID::NEG:
    return ~a;
  case TOKID::LO:
    return a & 255;
  case TOKID::HI:
    return (a >> 8) & 255;
  default:
    assert(0);
    return 0;
  }
}

static int64_t applyBinaryOp(TOKID::ID op, int64_t a, int64_t b) {
  switch(op) {
  case TOKID::ADD:
    return a + b;
  case TOKID::SUB:
    return a - b;
  case TOKID::MUL:
    return a * b;
  case TOKID::DIV:
    return a / b;
  case TOKID::EQU:
    return a == b;
  case TOKID::NEQ:
    return a != b;
  case TOKID::SHL:
    return a << b;
  case TOKID::SHR:
    return a >> b;
  case TOKID::AND:
    return a & b;
  case TOKID::OR:
    return a | b;
  case TOKID::XOR:
    return a ^ b;
  case TOKID::lOR:
    return a || b;
  case TOKID::lAND:
    return a && b;
  case TOKID::GT:
    return a > b;
  case TOKID::GE:
    return a >= b;
  case TOKID::LE:
    return a <= b;
  case TOKID::LT:
    return a < b;
  default:
    assert(0);
    return 0;
  }
}

void applyStackOp(bool& stack_error, std::stack<TOKID::ID>& ops, std::stack<int64_t>& values) {
  assert(!ops.empty());
  if (isunary(ops.top())) {
    if (values.empty()) {
      stack_error = true;
      ops.pop();
      return;
    }

    assert(!values.empty());
    auto val1 = values.top();
    values.pop();

    auto op = ops.top();
    ops.pop();

    values.push(applyUnaryOp(op, val1));
  } else {
    if (values.empty()) {
      stack_error = true;
      ops.pop();
      return;
    }

    assert(!values.empty());
    auto val2 = values.top(); 
    values.pop(); 
                  
    if (values.empty()) {
      stack_error = true;
      ops.pop();
      return;
    }

    assert(!values.empty());
    auto val1 = values.top(); 
    values.pop(); 

    auto op = ops.top();
    ops.pop();

    values.push(applyBinaryOp(op, val1, val2));
  }
}

// Adapted from: https://www.geeksforgeeks.org/expression-evaluation/

int64_t eval(Assembler* assembler, bool& parse_error, TokenList& tokens, int64_t pc, bool report_errors, bool consume_tokens) {
  std::stack<int64_t> values;
  std::stack<TOKID::ID> ops;
  bool stack_error = false;
  bool allow_unary = true;
  for (int i = 0; i < tokens.size() && !stack_error; ++i) {
    Token& token = tokens.at(i);
    if (token.isspace())
      continue;
    if (token.equals('#'))
      continue;
    if (tokens.at(i).equals(',')) {
      if (consume_tokens) {
        tokens = tokens.rest(i);
      }
      break;
    }

    TOKID::ID op = TOKID::NONE;
    if (token.classification() == Token::Operator) {
      op = getop(token);
    }

    if (allow_unary && op == TOKID::GT) {
      op = TOKID::HI;
    }
    if (allow_unary && op == TOKID::LT) {
      op = TOKID::LO;
    }
    if (allow_unary && op == TOKID::MUL) {
      op = TOKID::PC;
    }

    // Current token is an opening  
    // brace, push it to 'ops' 
    if (op == TOKID::LPAR) {
      ops.push(op);
      allow_unary = true;
    }

    // Current token is a number, push 
    // it to stack for numbers.
    else if (token.isnumber() || token.classification() == Token::Identifier || op == TOKID::PC) {
      int64_t val = -1;
      if (token.classification() == Token::Identifier) {
        Label* lbl = assembler->findLabel(token.toString().c_str());
        if (lbl) {
          val = lbl->m_Offset;
        } else {
          parse_error = true; // not really...
          auto msg = Hue::Util::String::static_printf("Undefined symbol '%s'", token.toString().c_str());
          if (report_errors) {
            assembler->recordError(msg, assembler->m_CurrentFileID, assembler->m_LineNumber, report_errors);
          }
          return -1;
        }
      } else if (op == TOKID::PC) {
        val = pc;
      } else {
        val = token.integerValue();
      }
      values.push(val);
      allow_unary = false;
    } 
    
    // Closing brace encountered, solve  
    // entire brace.     
    else if (op == TOKID::RPAR) {
      while (!ops.empty() && ops.top() != TOKID::LPAR) {
        applyStackOp(stack_error, ops, values);
      }
      ops.pop(); // '('
      allow_unary = false;
    }

    // operator
    else {
      assert(op >= 0);

      // While top of 'ops' has same or greater  
      // precedence to current token, which 
      // is an operator. Apply operator on top  
      // of 'ops' to top two elements in values stack. 
      while(!ops.empty() && precedence(ops.top()) >= precedence(op)) { 
        applyStackOp(stack_error, ops, values);
      } 
              
      // Push current token to 'ops'. 
      ops.push(op); 
      allow_unary = true;
    }
  }
  // Entire expression has been parsed at this 
  // point, apply remaining ops to remaining 
  // values. 
  while(!ops.empty()) { 
    applyStackOp(stack_error, ops, values);
  }
  if (stack_error && values.size() != 1) {
#ifdef _DEBUG
    Hue::Util::String errorline = tokens.join(" ");
#endif|
    parse_error = true;
    return -1;
  }
  return values.top();
}

int64_t Assembler::evaluateExpressionImpl(bool& parse_error, TokenList& tokens, int64_t pc, bool report_errors, bool consume_tokens) {
#ifdef _DEBUG
    Hue::Util::String sExpression = tokens.toString();
#endif
  parse_error = false;
  if (tokens.size() == 0) {
    recordError("empty expression", m_CurrentFileID, m_LineNumber, true);
    parse_error = true;
    return -1;
  }
  int64_t value = -1;
  int64_t operand = -1;
  Hue::Util::String labelname;
  if (isAnonymousLabelReference(labelname, tokens))
  { // anonymous label syntax encountered in rob hubbard source
    Label* label = findLabel(labelname);
    if (label)
    {
      operand = label->m_Offset;
      return operand;
    }
    else
    {
      if (report_errors) {
        recordError(Hue::Util::String::static_printf("Unknown identifier: %s", labelname.c_str()).c_str(), m_CurrentFileID, m_LineNumber, report_errors);
      }
      parse_error = true;
      return -1;
    }
  }
  return eval(this, parse_error, tokens, pc, report_errors, consume_tokens);
}

static Token
  s_CondIfToken(".if"),
  s_CondIfDefToken(".ifdef"),
  s_CondIfNDefToken(".ifndef"),
  s_CondElseToken(".else"),
  s_CondEndifToken(".endif");

bool Assembler::parseConditionals(TokenList& tokens)
{
  if (tokens.at(0).fmatch(s_CondIfToken)) {
    auto tmptokens = tokens.rest(1);
    int64_t condition = evaluateExpression(tmptokens, m_PC, true);
    if (condition < 0)
    {
      recordError("Error in conditional expression", m_CurrentFileID, m_LineNumber, true);
    }
    directiveIf(condition > 0);
    return true;
  }
  else if (tokens.at(0).fmatch(s_CondIfDefToken)) {
    if (tokens.size() > 1)
    {
      Label* dummy = findLabel(tokens.at(1).toString().c_str());
      directiveIf(dummy != NULL);
    }
    else
    {
      recordError("Error in conditional expression", m_CurrentFileID, m_LineNumber, true);
    }
    return true;
  }
  else if (tokens.at(0).fmatch(s_CondIfNDefToken)) {
    if (tokens.size() > 1)
    {
      Label* dummy = findLabel(tokens.at(1).toString().c_str());
      directiveIf(dummy == NULL);
    }
    else
    {
      recordError("Error in conditional expression", m_CurrentFileID, m_LineNumber, true);
    }
    return true;
  }
  else if (tokens.at(0).fmatch(s_CondElseToken)) {
    directiveElse();
    return true;
  }
  else if (tokens.at(0).fmatch(s_CondEndifToken)) {
    directiveEndif();
    return true;
  }
  return false;
}

void Assembler::parseConditionFalse(TokenList& tokens)
{
  parseConditionals(tokens);
}

void Assembler::directiveIf(bool condition)
{
  m_ConditionStack.push(condition && m_ConditionStack.top());
}

void Assembler::directiveElse()
{
  if (m_ConditionStack.size() <= 1)
  {
    recordError(".else without matching .if", m_CurrentFileID, m_LineNumber, true);
  }
  bool condition = !m_ConditionStack.top();
  m_ConditionStack.pop();
  m_ConditionStack.push(condition && m_ConditionStack.top());
}

void Assembler::directiveEndif()
{
  if (m_ConditionStack.size() <= 1)
  {
    recordError(".endif without matching .if", m_CurrentFileID, m_LineNumber, true);
  }
  m_ConditionStack.pop();
}

void Assembler::recordError(Hue::Util::String const& msg, InputFileID file_id, TextSpan const& span, bool record) {
  if (record) {
    m_Errors.push_back(Hue::Util::String::static_printf("%s(%d): error : %s", getInputFileName(file_id), span.m_Line, msg.c_str()));
  }
}

bool Assembler::isAnonymousLabelReference(Hue::Util::String& labelname, TokenList& tokens) {
  if (tokens.match_all('-')) {
    getAnonymousLabel(labelname, -1 * tokens.size(), '-');
    return true;
  } else if (tokens.match_all('+')) {
    //if (tokens.size() > 1) {
    //  int debug = 0;
    //}
    getAnonymousLabel(labelname, 1 * tokens.size() - 1, '+');
    return true;
  } else {
    labelname = "";
    return false;
  }
}

void Assembler::addLabel(const char* name, int64_t value) {
  auto label = findLabel(name);
  if (label) {
    recordError(Hue::Util::String::static_printf("Multiply defined symbol '%s' first defined in file '%s' line %d.\n", 
      name, 
      label->m_FileID, label->m_Span
    ).c_str(), m_CurrentFileID, m_LineNumber);
  } else {
    assert(findLabel(name) == NULL);
    Label* label = Label::create(name, value, m_CurrentFileID, m_LineNumber, m_CurrentSection->m_ID);
    m_Labels.insert(std::make_pair(name, label));
    assert(findLabel(name) != NULL);
  }
}

void Assembler::addLabel(Token const& token, int64_t value) {
  addLabel(token.toString().c_str(), value);
}

Label* Assembler::findLabel(const char* name) {
  Hue::Util::String sName;
  sName.create_wrapper(name);
  auto it = m_Labels.find(sName);
  if (it != m_Labels.end()) {
    return it->second;
  }
  return NULL;
}

int Assembler::getAnonLabelIndex(int offset, char suffix) {
  if (suffix == '+')
  {
    return m_AnonLabelIndexFwd + offset;
  }
  else if (suffix == '-')
  {
    return m_AnonLabelIndexBwd + offset;
  }
  else
  {
    assert(0);
    return 0; // INTERNAL ERROR!
  }
}

const char* Assembler::getAnonymousLabelName(Hue::Util::String& labelname, int offset, char suffix) {
  const char* suffixstr = suffix == '+' ? "fwd" : "bwd";
  labelname.printf("__anon__%d_%s", getAnonLabelIndex(offset, suffix), suffixstr);
  return labelname.c_str();
}

const char* Assembler::getAnonymousLabel(Hue::Util::String& labelname, int offset, char suffix) {
  return getAnonymousLabelName(labelname, offset, suffix);
}

void Assembler::addAnonymousLabel(int64_t value, char suffix) {
  Hue::Util::String name;
  getAnonymousLabelName(name, 0, suffix);
  addLabel(name.c_str(), value);
  if (suffix == '+')
    ++m_AnonLabelIndexFwd;
  else
    ++m_AnonLabelIndexBwd;
}

void Assembler::addUnresolved(const char* expression, int64_t m_PC, int64_t offset, AddrMode addrMode) {
  TokenList list;
  list.push_back(Token(expression, Token::Identifier));
  UnresolvedReference* unresolved = new UnresolvedReference(this->currentSection().m_ID, m_PC, offset, addrMode, m_CurrentFileID, m_LineNumber);
  unresolved->m_ExpressionTokens.push_back(Token(expression, Token::Identifier));
  m_Unresolved.push_back(unresolved);
}

void Assembler::addUnresolved(TokenList& expressiontokens, int64_t m_PC, int64_t offset, AddrMode addrMode)
{
  Hue::Util::String labelname;
  if (isAnonymousLabelReference(labelname, expressiontokens)) {
    addUnresolved(labelname.c_str(), m_PC, offset, addrMode);
  } else {
    UnresolvedReference* unresolved = new UnresolvedReference(this->currentSection().m_ID, m_PC, offset, addrMode, m_CurrentFileID, m_LineNumber);
    for (int i = 0; i < expressiontokens.size(); ++i) {
      unresolved->m_ExpressionTokens.push_back(expressiontokens.at(i));
    }
    m_Unresolved.push_back(unresolved);
  }
}

void Assembler::addUnresolvedLabel(const char* name, int64_t pc, TokenList& expressiontokens) {
  assert(findLabel(name) == NULL);
  UnresolvedReference* unresolved = new UnresolvedReference(this->currentSection().m_ID, pc, pc, AddrMode::IMPL, m_CurrentFileID, m_LineNumber);
  for (int i = 0; i < expressiontokens.size(); ++i) {
    unresolved->m_ExpressionTokens.push_back(expressiontokens.at(i));
  }
  unresolved->m_Label = name;
  m_Unresolved.push_back(unresolved);
}


void Assembler::writeInstruction(int opcode, InputFileID file_id, TextSpan const& span)
{
  if (m_CreateDebugInfo) {
    int debug = 0;
  }
  writeByte(opcode & 0xff);
}

void Assembler::writeByte(byte value)
{
  currentSection().m_Data.push_back(value);
  ++m_PC;
}

void Assembler::writeByteAt(byte value, int64_t position)
{
  assert(position >= 0);
  assert(position <= UINT_MAX);
  while (writeOffset() <= position) {
    writeByte(0);
  }
  currentSection().m_Data[(unsigned)position] = value;
}

void Assembler::writeWord(word value) {
  writeByte(value & 0xff);
  writeByte(value >> 8);
}

void Assembler::writeWordAt(word value, int64_t position) {
  writeByteAt(value & 0xff, position);
  writeByteAt(value >> 8, position + 1);
}

byte Assembler::calculateBranch(int64_t source, int64_t target, InputFileID file_id, TextSpan const& span) {
  int64_t offset = target - 2 - source;
  if (offset > 127 || offset < -128)
  {
    recordError(Hue::Util::String::static_printf("Branch out of range (%d)", offset).c_str(), file_id, span, true);
  }
  if (offset >= 0)
  {
    return (byte)offset;
  }
  else
  {
    return (byte)(256 + offset);
  }
} 

void Assembler::writeInstructionOperand(int64_t pc, int64_t offset, int64_t operand, AddrMode addrMode, InputFileID file_id, TextSpan const& span) {
  if (addrMode == AddrMode::REL)
  {
    byte branchvalue = calculateBranch(pc, operand, file_id, span);
    writeByteAt(branchvalue, offset);
  }
  else
  {
    int operand_size = OpcodeDefs::getOperandSize(addrMode);
    if (operand_size == 1)
    {
      if (operand > 255)
      {
        recordError(Hue::Util::String::static_printf("Value out of range: %d", operand), file_id, span, true);
      }
      writeByteAt((byte)operand, offset);
    }
    else if (operand_size == 2)
    {
      if (operand > 65535)
      {
        recordError(Hue::Util::String::static_printf("Value out of range: %d", operand), file_id, span, true);
      }
      writeByteAt(operand & 0xff, offset);
      writeByteAt((operand >> 8) & 0xff, offset + 1);
    }
  }
}

void                                              
Assembler::writePetsciiStringToMemory(Token const& token, bool isAscii) {
  auto unquoted = token.unquoted();
  int len = unquoted.length();
  for (int i = 0; i < len; ++i) {
    byte c = unquoted.at(i);
    if (isAscii && isalpha(c)) {
      c = ((c & 0x3f) ^ 0x20) | 0x40;
    }
    writeByte(c);
  }
}

void Assembler::parseConditionTrue(TokenList& tokens) {
#ifdef _DEBUG
  Hue::Util::String sTokens = tokens.join(" ");
  if (sTokens.contains("@w"))
  {
    int debug = 0;
  }
#endif
  while (!tokens.empty())
  {
    if (tokens.at(0).starts_with(".")) {
      TokenList expr;
      if (tokens.at(0).starts_with(".byt") || 
          tokens.at(0).equals(".word") || 
          tokens.at(0).equals(".text") || 
          tokens.at(0).equals(".null") ||
          tokens.at(0).equals(".asciiz"))
      {
        bool isNull = tokens.at(0).equals(".null") || tokens.at(0).equals(".asciiz");
        bool isAscii = false; // hrm..
        bool isByte = !tokens.at(0).equals(".word");
        int maxVal;
        if (isByte) {
          maxVal = 0xff;
        } else {
          maxVal = 0xffff;
        }
        for (int i = 1; i < tokens.size(); ++i)
        {
          if (tokens.at(i).equals(',')) {
            if (expr.empty())
              expr.push_back(Token("0", Token::Integer));
            int64_t val = evaluateExpression(expr, m_PC, false);
            if (val < 0)
            {
              addUnresolved(expr, m_PC, writeOffset(), isByte ? AddrMode::IMM : AddrMode::ABS);
            }
            else if (val > maxVal)
            {
              recordError(Hue::Util::String::static_printf("Value out of range: %s", expr.at(0).toString().c_str()), m_CurrentFileID, m_LineNumber, true);
            }
            if (isByte) {
              writeByte((byte)val);
            } else {
              writeWord((word)val);
            }
            expr.clear();
            continue;
          }
          else if (tokens.at(i).classification() == Token::String) {
            writePetsciiStringToMemory(tokens.at(i), isAscii);
          } else {
            expr.push_back(tokens.at(i));
          }
        }
        if (expr.size() > 0) {
          int64_t val = evaluateExpression(expr, m_PC, false);
          if (val < 0)
          {
            addUnresolved(expr, m_PC, writeOffset(), isByte ? AddrMode::IMM : AddrMode::ABS);
          }
          else if (val > maxVal)
          {
            recordError(Hue::Util::String::static_printf("Value out of range: %s", expr.at(0).toString().c_str()), m_CurrentFileID, m_LineNumber, true);
          }
          if (isByte) {
            writeByte((byte)val);
          } else {
            writeWord((word)val);
          }
          expr.clear();
        }
        if (isNull) {
          writeByte(0);
        }
        return;
      }
      else if (tokens.at(0).equals(".fill") || tokens.at(0).equals(".res")) {
        int64_t fill = 0;
        tokens = tokens.rest(1);
        auto count = evaluateExpression(tokens, m_PC, true, true);
        if (count > 0 && count < 65536) {
          if (tokens.size() > 1 && tokens.at(0).equals(',')) {
            auto tmptokens = tokens.rest(1);
            fill = evaluateExpression(tmptokens, m_PC, true, true);
          }
          if (fill >= 0) {
            while (count--) {
              writeByte((byte)fill);
            }
          }
        }
        return;
      }
      else if (tokens.at(0).equals(".assert")) {
        m_Assertions.push_back(new Assertion(this->currentSection().m_ID, m_PC, tokens.rest(1), m_CurrentFileID, m_LineNumber));
        return;
      }
      else if (tokens.at(0).equals(".org")) {
        auto tmptokens = tokens.rest(1);
        int64_t val = evaluateExpression(tmptokens, m_PC, true);
        if (val >= 0) {
          m_ORG = val;
          m_PC = val;
          if (findLabel("__START__") == NULL) {
            addLabel("__START__", val);
          }
        }
        return;
      }
      else if (tokens.at(0).equals(".obj")) {
        Hue::Util::String objname = tokens.at(1).toString();
        return;
      }
      else if (tokens.at(0).equals(".end")) {
        m_EndOfSource = true;
        return;
      }
      else if (parseConditionals(tokens)) {
        return;
      } else {
        recordError(Hue::Util::String::static_printf("Unsupported directive '%s'", tokens.at(0).toString().c_str()), m_CurrentFileID, m_LineNumber, true);
      }
    }
    if (tokens.at(0).equals('*')) {
      if (tokens.at(1).equals('=')) {
        auto tmptokens = tokens.rest(2);
        int64_t val = evaluateExpression(tmptokens, m_PC, true);
        if (val > 0) {
          if (m_ORG == 0) {
            m_ORG = val;
            m_PC = val;
            if (findLabel("__START__") == NULL) {
              addLabel("__START__", m_ORG);
            }
          }
          if (val < m_PC) {
            recordError("Cannot rewind program counter",  m_CurrentFileID, m_LineNumber, true);
            return;
          }
          while (val > m_PC) {
            writeByte(0);
          }
        }
      }
      return;
    }
    if (tokens.at(0).equals('-') || tokens.at(0).equals('+')) {
      addAnonymousLabel(m_PC, *tokens.at(0).m_pzTokenStart);
      tokens.consume(1);
      continue;
    }
    if (tokens.size() > 1)
    {
      if (tokens.at(1).equals(':')) {
        addLabel(tokens.at(0), m_PC);
        tokens.consume(2);
        continue;
      }
      else if (tokens.size() > 2 && tokens.at(1).equals('=')) {
        auto tmptokens = tokens.rest(2);
        int64_t value = evaluateExpression(tmptokens, m_PC, false);
        if (value >= 0) {
          addLabel(tokens.at(0), value);
        } else {
          addUnresolvedLabel(tokens.at(0).toString(), m_PC, tmptokens);
        }
        return;
      }
    }
    { // Must be assembly instruction or label
      std::vector<OpcodeDef*> const* opcodedefs = OpcodeDefs::checkIsInstruction(tokens.at(0));
      if (opcodedefs)
      {
        assert(!opcodedefs->empty());

        AddrMode
          addrMode;

        Op
          opcode = opcodedefs->at(0)->m_Op;

        bool
          noZP = false;

        bool
          isForceAbsolute = false;

        auto operands = tokens.rest(1);
        if (operands.size() == 0 || operands.at(0).equals('A') || operands.at(0).equals('a'))
        {
          addrMode = AddrMode::IMPL;
        }
        else
        {
          if (operands.at(0).equals("@w"))
          { // 64tass syntax
            operands.consume(1);
            isForceAbsolute = true;
          }
          if (operands.at(0).equals('|'))
          {
            noZP = true;
            operands = operands.rest(1);
          }
          if (operands.match_start("#"))
          {
            addrMode = AddrMode::IMM;
          }
          else if (operands.match_end("),Y"))
          {
            addrMode = AddrMode::INDY;
          }
          else if (operands.match_end(",X)"))
          {
            addrMode = AddrMode::XIND;
          }
          else if (operands.match_end(",X)"))
          {
            addrMode = AddrMode::XIND;
          }
          else if (operands.match_start("(") && operands.match_end(")"))
          {
            addrMode = AddrMode::IND;
          }
          else if (operands.match_end(",Y"))
          {
            addrMode = AddrMode::ABSY;
          }
          else if (operands.match_end(",X"))
          {
            addrMode = AddrMode::ABSX;
          }
          else
          {
            addrMode = AddrMode::ABS;
          }
        }
        if (OpcodeDefs::isBranchInstruction(opcode))
          addrMode = AddrMode::REL;
        else if (addrMode == AddrMode::ABS ||
          addrMode == AddrMode::ABSX ||
          addrMode == AddrMode::ABSY)
        {
          int64_t address = evaluateExpression(operands, m_PC, false);
#ifdef SASM_16BIT_TOKEN_OPERANDS_FORCES_ABSOLUTE_ADDRESSING
          if (operands.at(0).starts_with("$00") && operands.at(0).length() == 5)
          {
            isForceAbsolute = true;
          }
#endif
          if (!noZP)
          {
            bool forceZP = operands.at(0).equals('<');
            if ((address > 0 && address < 256 && !isForceAbsolute) || forceZP)
            {
              if (opcode != Op::JMP)
              {
                switch (addrMode)
                {
                  case AddrMode::ABS:
                    addrMode = AddrMode::ZP;
                    break;
                  case AddrMode::ABSX:
                    addrMode = AddrMode::ZPX;
                    break;
                  case AddrMode::ABSY:
                    addrMode = AddrMode::ZPY;
                    break;
                }
              }
            }
          }
        }
        auto def = OpcodeDefs::getOpcodeDefForAddressingMode(opcodedefs, addrMode);
        if (def == NULL)
        {
          recordError("Invalid addressing mode", m_CurrentFileID, m_LineNumber, true);
          return;
        }
		writeInstruction(def->m_Opcode, m_CurrentFileID, m_LineNumber);
        if (addrMode != AddrMode::IMPL)
        {
          int64_t operandValue = evaluateExpression(operands, m_PC - 1, false);
          if (operandValue < 0)
          {
            addUnresolved(operands, m_PC - 1, writeOffset(), addrMode);
            writeByte(0);
            if (OpcodeDefs::getOperandSize(addrMode) == 2)
            {
              writeByte(0);
            }
          }
          else
          {
            writeInstructionOperand(m_PC - 1, writeOffset(), operandValue, addrMode, m_CurrentFileID, m_LineNumber);
          }
        }
      }
      else if (m_AllowTASSLabels && isalpha(*tokens.at(0).m_pzTokenStart))
      {
        addLabel(tokens.at(0), m_PC);
        tokens.consume(1);
        continue;
      }
      else
      {
        recordError(Hue::Util::String::static_printf("Syntax Error: Unknown opcode '%s'", tokens.at(0).toString().c_str()), m_CurrentFileID, m_LineNumber, true);
      }
      return;
    }
  }
}

void Assembler::fprintErrors(FILE* file, int max_errors) {
  for (int i = 0; i < (int)m_Errors.size() && i < max_errors; ++i) {
    sasm_fprintf(file, "%s\n", m_Errors[i].c_str());
  }
  sasm_fprintf(file, "%d errors.\n", (int)m_Errors.size());
}

InputFileID Assembler::addFileInfo(const char* filename, const char* contents) {
  InputFileInfo info;
  info.m_Filename = filename;
#if SASM_ENABLE_DEBUGINFO
  info.m_Begin    = contents;
  info.m_End      = contents + strlen(contents);
#endif
  m_InputFiles.push_back(info);
  return (InputFileID)m_InputFiles.size();
}

bool                                              
Assembler::resolveTokenOrigin(int& inputfileID, int& linenumber, int& column_begin, int& column_end, Token const& token) {
#if SASM_ENABLE_DEBUGINFO
  for (int i = 0; i < (int)m_InputFiles.size(); ++i) {
    InputFileInfo& inp = m_InputFiles[i];
    if (token.m_pzTokenStart >= inp.m_Begin && token.m_pzTokenStart < inp.m_End) {
      assert(token.m_pzTokenEnd <= inp.m_End);
      if (inp.m_Lines.empty()) {
        const char* pzWork = inp.m_Begin;
        do {
          inp.m_Lines.push_back(pzWork);
          while (pzWork < inp.m_End && *pzWork != '\n') {
            ++pzWork;
          }
          if (pzWork < inp.m_End) {
            ++pzWork;
          }
        } while (pzWork < inp.m_End);
      }
      auto it = std::lower_bound(inp.m_Lines.begin(), inp.m_Lines.end(), token.m_pzTokenStart);
      assert(it != inp.m_Lines.end());
      inputfileID   = i;
      linenumber    = (int)(it - inp.m_Lines.begin()) + 1;
      column_begin  = (int)(token.m_pzTokenStart - *it);
      column_end = (int)(column_begin + token.m_pzTokenEnd - token.m_pzTokenStart);
      return true;
    }
  }
#else
  inputfileID = -1;
  linenumber  = 0;
  column      = 0;
#endif
  return false;
}

Assembler::InputFileInfo&                                    
Assembler::getInputFileInfo(int inputFileID) {
  --inputFileID;
  assert(inputFileID < (int)m_InputFiles.size());
  if (inputFileID < (int)m_InputFiles.size()) {
    return m_InputFiles[inputFileID];
  } else {
    static InputFileInfo 
      no_file;

    no_file.m_Filename  = "<no file>";
#if SASM_ENABLE_DEBUGINFO
    no_file.m_Begin     = "";
    no_file.m_End       = no_file.m_Begin;
#endif
    return no_file;
  }
}

const char* Assembler::getInputFileName(InputFileID file_id) {
  return getInputFileInfo(file_id).m_Filename.c_str();
}
}