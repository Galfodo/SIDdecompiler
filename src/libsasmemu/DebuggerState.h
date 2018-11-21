#ifndef SASM_DEBUGGERSTATE_H_INCLUDED
#define SASM_DEBUGGERSTATE_H_INCLUDED

#include "Types.h"
#include <string.h>
#include <assert.h>
#include <set>

namespace SASM {

#define SASM_MAX_UNISON_BREAKPOINTS 16 // Maximum number of breakpoints on the same address

class C64MachineState;
struct MemAccessMap;

struct TraceEvaluator {
  enum ReturnCode {
    CONTINUE  = 0,
    STOP      = 1
  };

  virtual ~TraceEvaluator() {}
  virtual int eval(C64MachineState&, int) {
    return TraceEvaluator::CONTINUE;
  }
};

// This is the set of evaluators for a given address
template<int MAX_EVALUATORS>
struct TraceEvaluatorSet {
  void add(TraceEvaluator* e) {
    for (int i = 0; i < MAX_EVALUATORS; ++i) {
      if (m_Evaluators[i] == NULL) {
        m_Evaluators[i] = e;
        ++m_Count;
        return;
      }
    }
    assert(0 && "too many evaluators");
  }

  int remove(TraceEvaluator* e) {
    for (int i = 0; i < MAX_EVALUATORS; ++i) {
      if (m_Evaluators[i] == e) {
        m_Evaluators[i] = NULL;
        --m_Count;
      }
    }
    return m_Count;
  }

  int invokeEvaluators(C64MachineState& state, int address, TraceEvaluator** last_evaluator) {
    auto remaining = m_Count;
    int retcode = TraceEvaluator::CONTINUE;
    for (int i = 0; remaining && i < MAX_EVALUATORS; ++i) {
      if (m_Evaluators[i] != NULL) {
        --remaining;
        int tmp = m_Evaluators[i]->eval(state, address);
        if (tmp != TraceEvaluator::CONTINUE) {
          retcode = tmp;
          if (last_evaluator) {
            *last_evaluator = m_Evaluators[i];
          }
          break;
        }
      }
    }
    return retcode;
  }

  TraceEvaluator* m_Evaluators[MAX_EVALUATORS];
  int             m_Count;
};

// This is a map of trace evaluators for a memory region
template<int MAX_EVALUATORS, int REGION_SIZE = 65536>
struct TraceEvaluatorMap {
  TraceEvaluatorMap() {
    memset(m_EvalMap, 0, sizeof(m_EvalMap));
  }

  ~TraceEvaluatorMap() {
  }

  inline bool evaluatorDefinedForAddress(int addr) {
    return m_EvalMap[addr].m_Count > 0;
  }

  void add(int addr, TraceEvaluator* e) {
    m_EvalMap[addr].add(e);
  }

  void remove(int addr, TraceEvaluator* e) {
    m_EvalMap[addr].remove(e);
  }

  int invokeEvaluators(C64MachineState& state, int addr, TraceEvaluator** last_evaluator) {
    int retval = m_EvalMap[addr].invokeEvaluators(state, addr, last_evaluator);
    return retval;
  }

  TraceEvaluatorSet<MAX_EVALUATORS> m_EvalMap[REGION_SIZE];
};

struct DebuggerState {
  DebuggerState();
  ~DebuggerState();

  enum TrapType {
    TRAP_STACK_UNDERFLOW  = (1 << 0),
    TRAP_STACK_OVERFLOW   = (1 << 1),
    TRAP_BRK              = (1 << 2),
    TRAP_READ             = (1 << 3),
    TRAP_WRITE            = (1 << 4),
    TRAP_EXECUTE          = (1 << 5),
    TRAP_HALT             = (1 << 6),
    TRAP_NONE             = 0,
    TRAP_ALL              = (TRAP_STACK_UNDERFLOW | TRAP_STACK_OVERFLOW | TRAP_BRK | TRAP_READ | TRAP_WRITE | TRAP_EXECUTE | TRAP_HALT)
  };

  inline const char* errorString() const {
    return m_ErrorString.c_str(); 
  }

  inline void incrementCPUCycles(int count) {
    m_CPUCycles += count;
  }

  inline void trap(TrapType type, int pc, int operand, TraceEvaluator* lastEvaluator) {
    if (m_Trapped == 0) {
      m_Trapped                 = type;
      m_TrappedPC               = pc;
      m_TrappedOperand          = operand; 
      m_TrappingTraceEvaluator  = lastEvaluator;
    }
  }

  inline void traceStackUnderflow(int pc, byte sp) {
    if (sp == 0xff) { // Called before POP, so sp has not yet wrapped around
      trap(TRAP_STACK_UNDERFLOW, pc, 0, NULL);
    }
  }

  inline void traceStackOverflow(int pc, byte sp) {
    if (sp == 0xff) { // Called after PUSH, so sp has wrapped around
      trap(TRAP_STACK_OVERFLOW, pc, 0, NULL);
    }
  }

  inline void traceBRK(int pc) {
  }

  inline void traceRead(int pc, int addr) {
    if (m_TraceReadMap->evaluatorDefinedForAddress(addr)) {
      assert(m_MachineState);
      TraceEvaluator* lastEvaluator = NULL;
      switch(m_TraceReadMap->invokeEvaluators(*m_MachineState, addr, &lastEvaluator)) {
      case TraceEvaluator::CONTINUE:
        break;
      default:
        trap(TRAP_READ, pc - 1, addr, lastEvaluator);
        break;
      }
    }
  }

  inline void traceWrite(int pc, int addr) {
    if (m_TraceWriteMap->evaluatorDefinedForAddress(addr)) {
      assert(m_MachineState);
      TraceEvaluator* lastEvaluator = NULL;
      switch(m_TraceWriteMap->invokeEvaluators(*m_MachineState, addr, &lastEvaluator)) {
      case TraceEvaluator::CONTINUE:
        break;
      default:
        trap(TRAP_WRITE, pc - 1, addr, lastEvaluator);
        break;
      }
    }
  }

  inline void traceExecute(int pc) {
    if (m_TraceExecuteMap->evaluatorDefinedForAddress(pc)) {
      assert(m_MachineState);
      TraceEvaluator* lastEvaluator = NULL;
      switch(m_TraceExecuteMap->invokeEvaluators(*m_MachineState, pc, &lastEvaluator)) {
      case TraceEvaluator::CONTINUE:
        break;
      default:
        trap(TRAP_EXECUTE, pc, 0, lastEvaluator);
        break;
      }
    }
  }

  inline void traceHalt(int addr) {
  }

  inline bool trapOccurred() const {
    return m_Trapped != 0;
  }

  inline void clearTrap() {
    m_Trapped = 0;
  }

  void setMachineState(C64MachineState* state) {
    m_MachineState = state;
  }

  void registerTraceExecuteEvaluator(TraceEvaluator* evaluator, int start_address, int count, bool manage) {
    for (int i = 0; i < count; ++i) {
      m_TraceExecuteMap->add(start_address + i, evaluator);
    }
    if (manage && m_ManagedEvaluators.count(evaluator) == 0) {
      m_ManagedEvaluators.insert(evaluator);
    }
  }

  void removeTraceExecuteEvaluator(TraceEvaluator* evaluator, int start_address, int count) {
    for (int i = 0; i < count; ++i) {
      m_TraceExecuteMap->remove(start_address + i, evaluator);
    }
    if (m_ManagedEvaluators.count(evaluator) > 0) {
      m_ManagedEvaluators.erase(evaluator);
    }
  }

  void registerTraceReadEvaluator(TraceEvaluator* evaluator, int start_address, int count, bool manage) {
    for (int i = 0; i < count; ++i) {
      m_TraceReadMap->add(start_address + i, evaluator);
    }
    if (manage && m_ManagedEvaluators.count(evaluator) == 0) {
      m_ManagedEvaluators.insert(evaluator);
    }
  }

  void removeTraceReadEvaluator(TraceEvaluator* evaluator, int start_address, int count) {
    for (int i = 0; i < count; ++i) {
      m_TraceReadMap->remove(start_address + i, evaluator);
    }
    if (m_ManagedEvaluators.count(evaluator) > 0) {
      m_ManagedEvaluators.erase(evaluator);
    }
  }

  void registerTraceWriteEvaluator(TraceEvaluator* evaluator, int start_address, int count, bool manage) {
    for (int i = 0; i < count; ++i) {
      m_TraceWriteMap->add(start_address + i, evaluator);
    }
    if (manage && m_ManagedEvaluators.count(evaluator) == 0) {
      m_ManagedEvaluators.insert(evaluator);
    }
  }

  void removeTraceWriteEvaluator(TraceEvaluator* evaluator, int start_address, int count) {
    for (int i = 0; i < count; ++i) {
      m_TraceWriteMap->remove(start_address + i, evaluator);
    }
    if (m_ManagedEvaluators.count(evaluator) > 0) {
      m_ManagedEvaluators.erase(evaluator);
    }
  }

  void                                            init();
  void                                            dumpState();
  void                                            dumpRegs();
  Hue::Util::String                               printRegs();
  void                                            setMemAccessMap(MemAccessMap* map);
  MemAccessMap*                                   memAccessMap() const { return m_MemAccessMap; }

  TraceEvaluatorMap<SASM_MAX_UNISON_BREAKPOINTS>* m_TraceExecuteMap;
  TraceEvaluatorMap<SASM_MAX_UNISON_BREAKPOINTS>* m_TraceReadMap;
  TraceEvaluatorMap<SASM_MAX_UNISON_BREAKPOINTS>* m_TraceWriteMap;
  int                                             m_Trapped;
  int                                             m_TrappedPC;
  int                                             m_TrappedOperand;
  TraceEvaluator*                                 m_TrappingTraceEvaluator;
  int                                             m_CPUCycles;
  C64MachineState*                                m_MachineState;
  Hue::Util::String                               m_ErrorString;
  std::set<TraceEvaluator*>                       m_ManagedEvaluators;
  MemAccessMap*                                   m_MemAccessMap;
};

}

#endif