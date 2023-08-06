
#include <string.h>

#include "C64MachineState.h"
#include "Assembler.h"
#include "DebugEvaluators.h"

namespace SASM {

template<> void incrementCPUCyclesT<true>(DebuggerState& db, int count) {
  db.incrementCPUCycles(count);
}

template<> void traceStackUnderflowT<true>(DebuggerState& db, int addr, byte sp) {
  db.traceStackUnderflow(addr, sp);
}

template<> void traceStackOverflowT<true>(DebuggerState& db, int addr, byte sp) {
  db.traceStackOverflow(addr, sp);
}

template<> void traceBRKT<true>(DebuggerState& db, int addr) {
  db.traceBRK(addr);
}

template<> void traceReadT<true>(DebuggerState& db, int pc, int addr) {
  db.traceRead(pc, addr);
}

template<> void traceWriteT<true>(DebuggerState& db, int pc, int addr) {
  db.traceWrite(pc, addr);
}

template<> void traceExecuteT<true>(DebuggerState& db, int addr) {
  db.traceExecute(addr);
}

template<> void traceHaltT<true>(DebuggerState& db, int addr) {
  db.traceHalt(addr);
}

void DebuggerState::init() {
  m_Trapped   = 0;
  m_TrappedPC = 0;
  m_CPUCycles = 0;
}

C64MachineState::C64MachineState() {
  m_Debugger.setMachineState(this);
  clearMem();
  softReset();
}

C64MachineState::~C64MachineState() {
}

void C64MachineState::clearMem() {
  memset(m_Mem, 0, sizeof(m_Mem));
  m_Mem[0x00] = 0x2f;
  m_Mem[0x01] = 0x37;
}

void C64MachineState::softReset() {
  int boot = getWord(0xfffc);
  initCPU(boot, 0, 0, 0);
}

void C64MachineState::initCPU(int addr, byte a, byte x, byte y) {
  A                       = a;
  X                       = x;
  Y                       = y;
  SP                      = 0xff;
  SR                      = 0;
  PC                      = addr;
  CurrentInstructionPC    = addr;
  m_Debugger.m_CPUCycles  = 0;
}

C64MachineState::CPUState C64MachineState::getCPUState() const {
  CPUState cpustate;
  cpustate.A  = this->A                   ;
  cpustate.X  = this->X                   ;
  cpustate.Y  = this->Y                   ;
  cpustate.SP = this->SP                  ;
  cpustate.SR = this->SR                  ;
  cpustate.PC = this->CurrentInstructionPC;
  return cpustate;
}

void C64MachineState::restoreCPUState(CPUState const& cpustate) {
  this->A                     = cpustate.A ;
  this->X                     = cpustate.X ;
  this->Y                     = cpustate.Y ;
  this->SP                    = cpustate.SP;
  this->SR                    = cpustate.SR;
  this->PC                    = cpustate.PC;
  this->CurrentInstructionPC  = cpustate.PC;
}

C64MachineState::Snapshot* C64MachineState::getSnapshot() const {
  auto snapshot = new Snapshot();
  snapshot->m_CPUState = getCPUState();
  memcpy(snapshot->m_Mem, this->m_Mem, sizeof(this->m_Mem));
  return snapshot;
}

void C64MachineState::restoreSnapshot(Snapshot* snapshot) {
  assert(snapshot);
  restoreCPUState(snapshot->m_CPUState);
  memcpy(this->m_Mem, snapshot->m_Mem, sizeof(this->m_Mem));
}

void C64MachineState::putBytes(int addr, byte* data, int count) {
  assert(count + addr < 65536);
  memcpy(m_Mem + addr, data, count);
}

void C64MachineState::putPRG(byte* data, int count, bool init_cpu) {
  assert(count >= 2);
  int addr = data[0] | ((int)data[1] << 8);
  putBytes(addr, data + 2, count - 2);
  if (init_cpu) {
    initCPU(addr);
  }
}

bool C64MachineState::parseAssemblerAssertions(Assembler& assembler) {
  for (int i = 0; i < assembler.assertions().size(); ++i) {
    Assertion* asm_assertion = assembler.assertions().at(i);
    TraceEvaluator* assertion = AssertEvaluator::parseAssertion(asm_assertion);
    if (assertion) {
      m_Debugger.m_TraceExecuteMap->add((int)asm_assertion->m_PC, assertion);
    } else {
      // report error?
    }
  }
  return true;
}

} // namespace SASM
