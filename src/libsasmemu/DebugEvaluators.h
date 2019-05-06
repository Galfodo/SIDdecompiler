#ifndef SASM_DEBUGEVALUATORS_H_INCLUDED
#define SASM_DEBUGEVALUATORS_H_INCLUDED

#include "DebuggerState.h"
#include "Assertion.h"

namespace SASM {

class Assembler;

struct DBExpression {
  virtual int eval(C64MachineState& state) = 0;
  virtual ~DBExpression() {}
};

namespace DebugEvaluators {
  struct PC : public DBExpression {
    virtual int eval(C64MachineState& state);
  };

  struct A : public DBExpression {
    virtual int eval(C64MachineState& state);
  };

  struct X : public DBExpression {
    virtual int eval(C64MachineState& state);
  };

  struct Y : public DBExpression {
    virtual int eval(C64MachineState& state);
  };

  struct SP : public DBExpression {
    virtual int eval(C64MachineState& state);
  };

  struct SR : public DBExpression {
    virtual int eval(C64MachineState& state);
  };

  struct BinaryOp : public DBExpression {
    BinaryOp(DBExpression* left, DBExpression* right);
    ~BinaryOp();
  protected:
    DBExpression* m_Left;
    DBExpression* m_Right;
  };

  struct OpEQ : public BinaryOp {
    OpEQ(DBExpression* left, DBExpression* right);
    virtual int eval(C64MachineState& state);
  };

  struct OpNE : public BinaryOp {
    OpNE(DBExpression* left, DBExpression* right);
    virtual int eval(C64MachineState& state);
  };

  struct Constant : public DBExpression {
    Constant(int constant);
    virtual int eval(C64MachineState& state);
    int m_Constant;
  };

  struct ReadMem : public DBExpression {
    ReadMem(int address); 
    virtual int eval(C64MachineState& state);
    int m_Address;
  };
}

struct AssertEvaluator : public TraceEvaluator {
                          AssertEvaluator(DBExpression* expr, Hue::Util::String const& text, InputFileID file_id, TextSpan const& span);
                          ~AssertEvaluator();
  virtual int             eval(C64MachineState& state, int addr) override;
  static AssertEvaluator* parseAssertion(Assertion* assertion);
private:
  DBExpression*           m_Expression;
  Hue::Util::String       m_Text;
  InputFileID             m_FileID;
  TextSpan                m_Span;
};

}

#endif
