
#include "DebuggerState.h"
#include "C64MachineState.h"
#include "Profiling.h"

namespace SASM {

DebuggerState::DebuggerState() :
  m_Trapped(0),
  m_TrappedPC(0),
  m_TrappedOperand(0),
  m_CPUCycles(0),
  m_MachineState(NULL),
  m_TraceReadMap(new TraceEvaluatorMap<SASM_MAX_UNISON_BREAKPOINTS>()),
  m_TraceWriteMap(new TraceEvaluatorMap<SASM_MAX_UNISON_BREAKPOINTS>()),
  m_TraceExecuteMap(new TraceEvaluatorMap<SASM_MAX_UNISON_BREAKPOINTS>())
{
}

DebuggerState::~DebuggerState() {
  delete m_TraceReadMap;
  delete m_TraceWriteMap;
  delete m_TraceExecuteMap;
  for (auto it = m_ManagedEvaluators.begin(); it != m_ManagedEvaluators.end(); ++it) {
    delete *it;
  }
}

void DebuggerState::setMemAccessMap(MemAccessMap* map) {
  m_MemAccessMap = map;
}

void DebuggerState::dumpRegs() {
  printf("%s", printRegs().c_str());
}

Hue::Util::String DebuggerState::printRegs() {
  assert(m_MachineState);
  Hue::Util::String sRegs;
  sRegs.printf( " PC  AC XR YR SP NV-BDIZC CYCLECOUNT\n");
  sRegs.appendf("%04x %02x %02x %02x %02x %d%d%d%d%d%d%d%d %10d\n",
    m_MachineState->PC,
    m_MachineState->A,
    m_MachineState->X,
    m_MachineState->Y,
    m_MachineState->SP,
    m_MachineState->SR & C64MachineState::FN ? 1 : 0,
    m_MachineState->SR & C64MachineState::FV ? 1 : 0,
    m_MachineState->SR & C64MachineState::FU ? 1 : 0,
    m_MachineState->SR & C64MachineState::FB ? 1 : 0,
    m_MachineState->SR & C64MachineState::FD ? 1 : 0,
    m_MachineState->SR & C64MachineState::FI ? 1 : 0,
    m_MachineState->SR & C64MachineState::FZ ? 1 : 0,
    m_MachineState->SR & C64MachineState::FC ? 1 : 0,
    m_CPUCycles
  );
  return sRegs;
}

void DebuggerState::dumpState() {
  printf("\nRegister dump:\n");
  dumpRegs();
}

} // namespace SASM
