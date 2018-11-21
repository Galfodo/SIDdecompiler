
#include "DebugEvaluators.h"
#include "C64MachineState.h"
#include "DebuggerState.h"

#include <stack>
#include <vector>

namespace SASM {

namespace DebugEvaluators {

int PC::eval(C64MachineState& state) {
  return state.PC;
}

int A::eval(C64MachineState& state) {
  return state.A;
}

int X::eval(C64MachineState& state) {
  return state.X;
}

int Y::eval(C64MachineState& state) {
  return state.Y;
}

int SP::eval(C64MachineState& state) {
  return state.SP;
}

int SR::eval(C64MachineState& state) {
  return state.SR;
}

BinaryOp::BinaryOp(DBExpression* left, DBExpression* right) : m_Left(left), m_Right(right) {
  assert(left);
  assert(right);
}

BinaryOp::~BinaryOp() {
  delete m_Left;
  delete m_Right;
}

OpEQ::OpEQ(DBExpression* left, DBExpression* right) : BinaryOp(left, right) {
}

int OpEQ::eval(C64MachineState& state) {
  return m_Left->eval(state) == m_Right->eval(state);
}

OpNE::OpNE(DBExpression* left, DBExpression* right) : BinaryOp(left, right) {
}

int OpNE::eval(C64MachineState& state) {
  return m_Left->eval(state) != m_Right->eval(state);
}

Constant::Constant(int constant) : m_Constant(constant) {
}

int Constant::eval(C64MachineState& state) {
  return m_Constant;
}

ReadMem::ReadMem(int addr) : m_Address(addr) {
}

int ReadMem::eval(C64MachineState& state) {
  return state.m_Mem[m_Address];
}

}

enum TokType {
  TOK_EQ,
  TOK_NE,
  TOK_LPAR,
  TOK_RPAR,
  TOK_LBR,
  TOK_RBR,
  TOK_NUMBER,
  TOK_IDENTIFIER
};

struct ExprToken {
  ExprToken(TokType type) : m_Type(type), m_IntegerValue(0) {}
  explicit ExprToken(int64_t value) : m_Type(TOK_NUMBER), m_IntegerValue(value) {}
  explicit ExprToken(Hue::Util::String const& identifier) : m_Type(TOK_IDENTIFIER), m_Identifier(identifier), m_IntegerValue(0) {}
  TokType           m_Type;
  int64_t           m_IntegerValue;
  Hue::Util::String m_Identifier;
};

static bool convertTokenList(std::vector<ExprToken>& convertedTokens, TokenList const& tokens) {
  for (int i = 0; i < tokens.size(); ++i) {
    Token const& tok = tokens.at(i);
    if (tok.classification() == Token::Whitespace) {
      continue;
    } else if (tok.classification() == Token::Operator) {
      if      (tok.equals("=="))         { convertedTokens.push_back(TOK_EQ   ); }
      else if (tok.equals("!="))         { convertedTokens.push_back(TOK_NE   ); }
      else if (tok.equals('('))          { convertedTokens.push_back(TOK_LPAR ); }
      else if (tok.equals(')'))          { convertedTokens.push_back(TOK_RPAR ); }
      else if (tok.equals('['))          { convertedTokens.push_back(TOK_LBR  ); }
      else if (tok.equals(']'))          { convertedTokens.push_back(TOK_RBR  ); }
      else {
        // unknown operator
        return false;
      }
    } else if (tok.isnumber()) {
      convertedTokens.push_back(ExprToken(tok.integerValue()));
    } else if (tok.classification() == Token::Identifier) {
      convertedTokens.push_back(ExprToken(tok.toString()));
    }
  }
  return true;
}

template<typename T>
static bool createBinaryExpression(std::stack<DBExpression*>& valueStack) {
  if (valueStack.size() >= 2) {
    DBExpression *right = valueStack.top(); valueStack.pop();
    DBExpression *left  = valueStack.top(); valueStack.pop();
    DBExpression *expr  = new T(left, right);
    valueStack.push(expr);
    return true;
  }
  return false;
}

static DBExpression*
parseTokenList(TokenList const& tokens) {
  std::vector<ExprToken> parserTokens;
  if (convertTokenList(parserTokens, tokens)) {
    std::stack<ExprToken> opStack;
    std::stack<DBExpression*> valueStack;
    for (int i = 0; i < parserTokens.size(); ++i) {
      ExprToken const& token = parserTokens.at(i);
      switch (token.m_Type) {
      case TOK_IDENTIFIER:
        {
          if      (token.m_Identifier.equal("A"))     { valueStack.push(new DebugEvaluators::A());     }
          else if (token.m_Identifier.equal("X"))     { valueStack.push(new DebugEvaluators::X());     }
          else if (token.m_Identifier.equal("Y"))     { valueStack.push(new DebugEvaluators::Y());     }
          else if (token.m_Identifier.equal("PC"))    { valueStack.push(new DebugEvaluators::PC());    }
          else if (token.m_Identifier.equal("SP"))    { valueStack.push(new DebugEvaluators::SP());    }
          else if (token.m_Identifier.equal("SR")) { valueStack.push(new DebugEvaluators::SR()); }
          else {
            // Unknown identifier
            return NULL;
          }
        }
        break;
      case TOK_NUMBER:
        valueStack.push(new DebugEvaluators::Constant((int)token.m_IntegerValue));
        break;
      case TOK_LBR:
        {
          if (i + 2 < parserTokens.size()) {
            if (parserTokens[i + 1].m_Type == TOK_NUMBER && parserTokens[i + 2].m_Type == TOK_RBR) {
              valueStack.push(new DebugEvaluators::ReadMem((int)parserTokens[i + 1].m_IntegerValue));
              i += 2;
            } else {
              return NULL;
            }
          } else {
            return NULL;
          }
        }
        break;
      default:
        opStack.push(token);
        break;
      }
    }
    while (!opStack.empty()) {
      ExprToken token = opStack.top();
      opStack.pop();
      switch(token.m_Type) {
      case TOK_EQ:
        if (!createBinaryExpression<DebugEvaluators::OpEQ>(valueStack)) {
          return NULL;
        }
        break;
      case TOK_NE:
        if (!createBinaryExpression<DebugEvaluators::OpNE>(valueStack)) {
          return NULL;
        }
        break;
      default:
        return NULL;
      }
    }
    if (valueStack.size() == 1) {
      return valueStack.top();      
    }
  }
  return NULL;
}

AssertEvaluator::AssertEvaluator(DBExpression* expr, Hue::Util::String const& text, Hue::Util::String const& filename, int line) : 
  m_Expression(expr), 
  m_Text(text), 
  m_FileName(filename), 
  m_Line(line) {
  assert(expr);
  if (m_FileName.empty()) {
    m_FileName = "<stdin>";
  }
}

AssertEvaluator::~AssertEvaluator() {
  delete m_Expression;
}

int AssertEvaluator::eval(C64MachineState& state, int addr) {
  if (m_Expression->eval(state)) {
    return TraceEvaluator::CONTINUE;
  } else {
    state.m_Debugger.m_ErrorString = Hue::Util::String::static_printf("ASSERTION FAILURE at memory location $%04x.\n  Source file '%s', line %d.\n  Expression: '%s'", (int)addr, m_FileName.c_str(), m_Line, m_Text.c_str());
    return TraceEvaluator::STOP;
  }
}

AssertEvaluator* AssertEvaluator::parseAssertion(Assertion* assertion) {
  assert(assertion);
  DBExpression* expr = parseTokenList(assertion->m_ExpressionTokens);
  if (expr) {
    return new AssertEvaluator(expr, assertion->m_ExpressionTokens.toString(), assertion->m_FileName, assertion->m_Line);
  }
  return NULL;
}

}
