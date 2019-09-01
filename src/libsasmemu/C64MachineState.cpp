
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

MemoryMappedDevice::MemoryMappedDevice() : m_MachineState(nullptr) {
}

MemoryMappedDevice::~MemoryMappedDevice() {
}

void MemoryMappedDevice::onAttach(C64MachineState& machine) {
  m_MachineState = &machine;
}

void DebuggerState::init() {
  m_Trapped   = 0;
  m_TrappedPC = 0;
  m_CPUCycles = 0;
}

MemConfig::MemConfig() {
  for (int i = 0; i < 256; ++i) {
    m_PageMask[i]   = 0xff;
    m_PageTable[i]  = nullptr;
  }
};

void MemConfig::init(byte* ram_base) {
  assert(ram_base);
  for (int i = 0; i < 256; ++i) {
    m_PageMask[i]   = 0xff;
    m_PageTable[i]  = ram_base + 256 * i;
  }
}

void MemConfig::setPageConfig(int page_index, byte* page, byte pagemask) {
  assert(page_index >= 0 && page_index < 256);
  m_PageMask[page_index]  = pagemask;
  m_PageTable[page_index] = page;
}

C64MachineState::C64MachineState() : 
  m_CurrentReadConfig(m_ReadConfig[MEM_DEFAULT_CONFIGURATION]), 
  m_CurrentWriteConfig(m_WriteConfig[MEM_DEFAULT_CONFIGURATION]) 
{
  for (int i = 0; i < MEM_CONFIGURATIONS; ++i) {
    m_ReadConfig[i].init(m_Mem);
    m_WriteConfig[i].init(m_Mem);
  }
  m_Debugger.setMachineState(this);
  clearMem();
  softReset();
}

C64MachineState::~C64MachineState() {
}

void C64MachineState::attach(MemoryMappedDevice* device) {
  m_Devices.push_back(std::unique_ptr<MemoryMappedDevice>(device));
  device->onAttach(*this);
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

void C64MachineState::clearIRQ() {
  IRQ = false;
}

void C64MachineState::setIRQ() {
  IRQ = true;
}

void C64MachineState::clearNMI() {
  NMI = false;
  _NMI = false;
}

void C64MachineState::setNMI() {
  NMI = true;
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
  BRK                     = false;
  clearIRQ();
  clearNMI();
}

C64MachineState::CPUState C64MachineState::getCPUState() const {
  CPUState cpustate;
  cpustate.A    = this->A                   ;
  cpustate.X    = this->X                   ;
  cpustate.Y    = this->Y                   ;
  cpustate.SP   = this->SP                  ;
  cpustate.SR   = this->SR                  ;
  cpustate.PC   = this->CurrentInstructionPC;
  cpustate.BRK  = this->BRK                 ;
  cpustate.IRQ  = this->IRQ                 ;
  cpustate.NMI  = this->NMI                 ;
  cpustate._NMI = this->_NMI                ;
  return cpustate;
}

void C64MachineState::restoreCPUState(CPUState const& cpustate) {
  this->A                     = cpustate.A   ;
  this->X                     = cpustate.X   ;
  this->Y                     = cpustate.Y   ;
  this->SP                    = cpustate.SP  ;
  this->SR                    = cpustate.SR  ;
  this->PC                    = cpustate.PC  ;
  this->CurrentInstructionPC  = cpustate.PC  ;
  this->BRK                   = cpustate.BRK ;
  this->IRQ                   = cpustate.IRQ ; 
  this->NMI                   = cpustate.NMI ; 
  this->_NMI                  = cpustate._NMI;
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

void C64MachineState::load(int addr, byte const* data, int count, bool init_cpu) {
  assert(count + addr < 65536);
  memcpy(m_Mem + addr, data, count);
  if (init_cpu) {
    initCPU(addr);
  }
}

void C64MachineState::loadPRG(byte const* data, int count, bool init_cpu) {
  assert(count >= 2);
  int addr = data[0] | ((int)data[1] << 8);
  load(addr, data + 2, count - 2, init_cpu);
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

}
