
#include "TraceOperand.h"
#include "DebuggerState.h"
#include "OpcodeDefs.h"
#include "C64MachineState.h"
#include "Profiling.h"

namespace SASM {

TraceNodePtr
  TraceNode::s_Unresolvable,
  TraceNode::s_Null;

#if SASM_DEBUG_TRACESTATE
int64_t 
  TraceNode::s_CreateCount,
  TraceNode::s_AliveCount,
  TraceNode::s_MaxAliveCount;
#endif

void OperandTraceState::addTraceNodePair(TraceNode* lo, TraceNode* hi) {
  assert(lo);
  assert(hi);
  m_TraceNodePairs.insert(std::make_pair(lo, hi));
}

void OperandTraceState::dumpStats() {
#if SASM_DEBUG_TRACESTATE
  Hue::Util::String sStats;
  sStats.printf("TraceNode stats:\n");
  sStats.appendf("  Total allocation count :      %lld\n", TraceNode::s_CreateCount);
  sStats.appendf("  Max alive count :             %lld\n", TraceNode::s_MaxAliveCount);
  sStats.appendf("  Current alive count :         %lld\n", TraceNode::s_AliveCount);
  sStats.appendf("\n");
  sStats.appendf("  Unresolved address operands:  %d\n", (int)m_Unresolvable.size());
  sStats.appendf("  Relocatable address operands: %d\n", (int)m_RelocPairs.size());
  //for (auto it = m->m_RelocPairs.begin(); it  != m->m_RelocPairs.end(); ++it) {
  //  printf("%04x:%04x\n", it->first, it->second);
  //}
  printf("\n%s\n", sStats.c_str());
#endif
}

bool AddressOperandTrace::isResolvable(OperandTracerEmu& emu, int addr, int regionsize) {
  if (m_Lo.get() == NULL && m_Hi.get() == NULL) {
    return true;
  } else if (m_Lo.get() && m_Hi.get()) {
    return m_Lo.get()->isResolvable(emu, addr, regionsize) && m_Hi.get()->isResolvable(emu, addr, regionsize);
  } else {
    return false;
  }
}

struct OperandTracer : public TraceEvaluator {
  OperandTracer(OperandTracerEmu& emu, OperandTraceState* opTraceState, int addr, int regionsize) : m_OperandTraceState(opTraceState), m_RegionAddress(addr), m_RegionSize(regionsize), m_Emu(emu) {
  }

  void addUnresolvable(AddressOperandTrace const& tr) {
    auto it = m_OperandTraceState->m_Unresolvable.find(tr.m_OpAddr);
    if (it == m_OperandTraceState->m_Unresolvable.end()) {
      auto instance = new AddressOperandTrace(tr);
      m_OperandTraceState->m_Unresolvable.insert(std::make_pair(tr.m_OpAddr, AddressOperandTracePtr(instance)));
    }
  }

  void getOrCreateAddressTraceABS(int opaddr, bool forceCreate) {
    TraceNode* lo = m_OperandTraceState->m_CurrentTraces[opaddr + 0].get();
    TraceNode* hi = m_OperandTraceState->m_CurrentTraces[opaddr + 1].get();
    if ((lo == NULL) != (hi == NULL)) {
      int eff = m_Emu.getWord(opaddr);
      if (eff >= m_RegionAddress && eff < m_RegionAddress + m_RegionSize) {
        m_OperandTraceState->m_IncompleteOperands.insert(opaddr);
      }
      if (lo == NULL) {
        m_OperandTraceState->m_CurrentTraces[opaddr + 0] = hi = TraceNode::create(opaddr + 0, NULL, NULL);
      } else {
        m_OperandTraceState->m_CurrentTraces[opaddr + 1] = hi = TraceNode::create(opaddr + 1, NULL, NULL);
      }
    }
    if (lo == NULL && hi == NULL) {
      if (forceCreate) {
        lo = TraceNode::create(opaddr, NULL, NULL);
        hi = TraceNode::create(opaddr + 1, NULL, NULL);
        m_OperandTraceState->m_CurrentTraces[opaddr + 0] = lo;
        m_OperandTraceState->m_CurrentTraces[opaddr + 1] = hi;
      } else {
        return;
      }
    }
    AddressOperandTrace tr(opaddr, lo, hi);
    if (forceCreate || !tr.isNoTrace()) {
      if (tr.isResolvable(m_Emu, m_RegionAddress, m_RegionSize)) {
        m_OperandTraceState->addTraceNodePair(lo, hi);
      } else {
        addUnresolvable(tr);
      }
    }
  }

  OperandTraceState*  m_OperandTraceState;
  int                 m_RegionAddress;
  int                 m_RegionSize;
  OperandTracerEmu&   m_Emu;
};

struct ExecuteTracer2 : OperandTracer {
  ExecuteTracer2(OperandTracerEmu& emu, OperandTraceState* opTraceState, int addr, int regionsize) : OperandTracer(emu, opTraceState, addr, regionsize) {
  }

  int eval(C64MachineState& emu_, int addr) override {
    OperandTracerEmu& emu = (OperandTracerEmu&)emu_;
    //if (addr == 0xcd2a) {
    //  int debug = 0;
    //}
    ++emu.PC; // Temporarily increment PC to allow use of emulator effective address calculation
    auto m                = m_OperandTraceState;
    byte opcode           = emu.getByte(addr);
    OpcodeDef* def        = OpcodeDefs::getOpcodeDef(opcode);
    int opaddr            = addr + 1;
    int opsize            = OpcodeDefs::getOperandSize(opcode);
    int zp                = 0;
    TraceNodePtr* src     = NULL;
    TraceNodePtr* dst     = NULL;
    TraceNode* prevRight  = NULL;
    int effectiveAddr     = -1;

    // Set up source and destination for operations:
    switch (def->m_Op) {
    // Target: Registers
    case Op::PLA:
    case Op::LDA:
      dst = &m->m_A;
      break;
    case Op::LDX:
      dst = &m->m_X;
      break;
    case Op::LDY:
      dst = &m->m_Y;
      break;
    case Op::ADC:
      src = dst = &m->m_A;
      break;

    // Source: registers
    case Op::PHA:
    case Op::STA:
      src = &m->m_A;
      break;
    case Op::STX:
      src = &m->m_X;
      break;
    case Op::STY:
      src = &m->m_Y;
      break;

    // Between registers
    case Op::TAX:
      src = &m->m_A;
      dst = &m->m_X;
      break;
    case Op::TAY:
      src = &m->m_A;
      dst = &m->m_Y;
      break;
    case Op::TXA:
      src = &m->m_X;
      dst = &m->m_A;
      break;
    case Op::TYA:
      src = &m->m_Y;
      dst = &m->m_A;
      break;

    // Register modifying
    case Op::DEX:
    case Op::INX:
      src = dst = &m->m_X;
      break;
    case Op::DEY:
    case Op::INY:
      src = dst = &m->m_Y;
      break;
    case Op::JMP:
    case Op::JSR:
    case Op::RTS:
    case Op::RTI:
      break;
    default:
    // We don't care about any other instructions
      goto done;
    }

    if (src) {
      if (src->get() && src->get()->maxdepth() > 32) {
        *src = TraceNode::getUnresolvable();
      }
    }

    // Calculate effective address
    switch(def->m_AddrMode) {
    case AddrMode::ABS:
      effectiveAddr = emu.ABSOLUTE();
      break;
    case AddrMode::ABSX:
      effectiveAddr = emu.ABSOLUTEX();
      break;
    case AddrMode::ABSY:
      effectiveAddr = emu.ABSOLUTEY();
      break;
    case AddrMode::IND:
      {
        int realOpAddr = emu.getWord(opaddr);
        getOrCreateAddressTraceABS(realOpAddr, true);
        effectiveAddr = emu.getWord(realOpAddr);
      }
      break;
    case AddrMode::INDY:
      effectiveAddr = emu.INDIRECTY();
      break;
    case AddrMode::XIND:
      effectiveAddr = emu.INDIRECTX();
      break;
    case AddrMode::REL:
      {
        byte tmp = emu.getByte(opaddr);
        effectiveAddr = (tmp < 0x80) ? emu.PC + tmp : emu.PC + tmp - 0x100;
      }
      break;
    case AddrMode::ZP:
      effectiveAddr = emu.ZEROPAGE();
      break;
    case AddrMode::ZPX:
      effectiveAddr = emu.ZEROPAGEX();
      break;
    case AddrMode::ZPY:
      effectiveAddr = emu.ZEROPAGEY();
      break;
    }
    if (def->m_AddrMode == AddrMode::IMM) {
      switch (def->m_Op) {
      case Op::LDA:
      case Op::LDX:
      case Op::LDY:
        if (m->m_CurrentTraces[opaddr].get() == NULL) {
          m->m_CurrentTraces[opaddr] = TraceNode::create(opaddr, NULL, NULL);
        }
        assert(dst);
        *dst = TraceNode::create(opaddr, m->m_CurrentTraces[opaddr].get(), NULL);
        goto done;
      case Op::ADC:
        if (m->m_CurrentTraces[opaddr].get() == NULL) {
          m->m_CurrentTraces[opaddr] = TraceNode::create(opaddr, NULL, NULL);
        }
        *dst = TraceNode::create(opaddr, m->m_A.get(), m->m_CurrentTraces[opaddr].get());
        goto done;
      default:
        m->m_A = TraceNode::getUnresolvable();
        break;
      }
      goto done;
    }
    switch (def->m_Op) {
    case Op::JMP:
      break;
    case Op::JSR:
      m->m_Stack.push_back(TraceNode::getUnresolvable());
      m->m_Stack.push_back(TraceNode::getUnresolvable());
      break;
    case Op::RTS:
      if (m->m_Stack.size() >= 2) {
        auto lo = m->m_Stack.back(); m->m_Stack.pop_back();
        auto hi = m->m_Stack.back(); m->m_Stack.pop_back();
        assert(lo.get());
        assert(hi.get());
        m->addTraceNodePair(lo.get(), hi.get());
      } else {
        // error
        //int debug = 0;
      }
      goto done;

    // Read:
    case Op::PLA:
      assert(dst);
      if (m->m_Stack.empty()) {
        *dst = TraceNode::getUnresolvable();
      } else {
        *dst = m->m_Stack.back();
        m->m_Stack.pop_back();
      }
      goto done;
    case Op::ADC:
      prevRight = m->m_A.get();
    case Op::LDA:
    case Op::LDY:
    case Op::LDX:
      if (def->m_AddrMode == AddrMode::INDY || def->m_AddrMode == AddrMode::XIND) {
        zp = emu.getByte(opaddr);
        getOrCreateAddressTraceABS(zp, true);
      }
      assert(dst);
      assert(effectiveAddr >= 0);
      *dst = TraceNode::create(effectiveAddr, m->m_CurrentTraces[effectiveAddr].get(), prevRight);
      assert(dst->refCount() == 1);
      break;

    // Write:
    case Op::STA:
    case Op::STX:
    case Op::STY:
      assert(src);
      assert(effectiveAddr >= 0);
      m->m_CurrentTraces[effectiveAddr] = *src;
      break;
    case Op::PHA:
      assert(src);
      if (m->m_Stack.size() == 256) {
        m->m_Stack.pop_front();
      }
      m->m_Stack.push_back(*src);
      goto done;
    default:
      assert(src);
      assert(dst);
      *dst = *src;
      goto done;
    }
    if (opsize == 2) {
      getOrCreateAddressTraceABS(opaddr, false);
    } else {
      assert(opsize == 1);
      //getOrCreateAddressTraceZP(opaddr, false);
    }
done:
    --emu.PC; // ... and restore it
    return TraceEvaluator::CONTINUE;
  }
};

OperandTraceState::OperandTraceState() {
  init();
}

OperandTraceState::~OperandTraceState() {
  cleanup();
}

void OperandTraceState::addRelocAddr(std::pair<int, int> lohi) {
  if (lohi.first < 0 || lohi.second < 0) {
    return;
  }
  if (m_RelocPairs.find(lohi) == m_RelocPairs.end()) {
    m_RelocPairs.insert(lohi);
  }
}

TraceNode* OperandTraceState::lookupOrigin(int addr) {
  return m_CurrentTraces[addr].get();
}

void OperandTraceState::buildRelocationTables(OperandTracerEmu& emu, int relocRangeStart, int relocRangeSize, bool bridgeGaps) {
  printf("TraceNode pairs: %d\n", (int)m_TraceNodePairs.size());
  for (auto it = m_TraceNodePairs.begin(); it != m_TraceNodePairs.end(); ++it) {

    auto pcLoNode = it->first.get();
    auto pcHiNode = it->second.get();

    assert(pcLoNode);
    assert(pcHiNode);

    std::set<int> loNodes;
    std::set<int> hiNodes;
    pcLoNode->getLeafNodes(loNodes);
    pcHiNode->getLeafNodes(hiNodes);
    auto m = emu.debugger().memAccessMap();
    assert(m);
    if (loNodes.size() > 1 || hiNodes.size() > 1) {
      for (auto it = loNodes.begin(); it != loNodes.end(); ) {
        auto node = *it;
        assert(m->getAccessType(node) != MemAccessMap::UNTOUCHED);
        if (node < relocRangeStart || node >= relocRangeStart + relocRangeSize) {
          it = hiNodes.erase(it);
        } else  if (m->getAccessType(node) & MemAccessMap::WRITE) {
          it = loNodes.erase(it);
        } else {
          ++it;
        }
      }
      for (auto it = hiNodes.begin(); it != hiNodes.end(); ) {
        auto node = *it;
        assert(m->getAccessType(node) != MemAccessMap::UNTOUCHED);
        if (node < relocRangeStart || node >= relocRangeStart + relocRangeSize) {
          it = hiNodes.erase(it);
        } else if (m->getAccessType(node) & MemAccessMap::WRITE) {
          it = hiNodes.erase(it);
        } else {
          ++it;
        }
      }
    }
    if (loNodes.size() > 0 && hiNodes.size() > 0) {
      int bestdist  = 0x10000;
      int besthi    = 0;
      int bestlo    = 0;
      for (auto ilo = loNodes.begin(); ilo != loNodes.end(); ++ilo) {
        for (auto ihi = hiNodes.begin(); ihi != hiNodes.end(); ++ihi) {
          int delta = abs(*ihi - *ilo);
          if (delta <= bestdist) {
            int testaddr = emu.getByte(*ilo) | (emu.getByte(*ihi)  << 8);
            if (testaddr >= relocRangeStart && testaddr <= relocRangeStart + relocRangeSize) {
              bestdist = delta;
              besthi = *ihi;
              bestlo = *ilo;
            }
            else
            {
              //int debug = 0;
            }
          }
        }
      }
      int originLo = bestlo; //m_Lo.get()->resolveOrigin(emu, addr, regionsize);
      int originHi = besthi; //m_Hi.get()->resolveOrigin(emu, addr, regionsize);
      if (originLo > 0 && originHi > 0) {
        //if (1){ //(m_Lo.get()->leafNodes() > 1 || m_Hi.get()->leafNodes() > 1) {
        //  if (m_Lo.get()->address() < 0x100) {
        //    printf("%04x:%04x : ORG (%04x:%04x) DEPTH (%3d:%3d) BREADTH (%3d:%3d) \n", m_Lo.get()->address(), m_Hi.get()->address(), originLo, originHi, m_Lo.get()->maxdepth(), m_Hi.get()->maxdepth(), m_Lo.get()->leafNodes(), m_Hi.get()->leafNodes());
        //  }
        //}
        //int effectiveaddr = emu.m_Mem[originLo] | (emu.m_Mem[originHi] << 8);
        //if (effectiveaddr < 0xd000 || effectiveaddr >= 0xe000) {
          addRelocAddr(std::make_pair(originLo, originHi));
        //} else {
        //  int debug = 0;
        //}
      }
      else
      {
        //return std::make_pair<int, int>(-1, -1);
      }
    } else {
      //return std::make_pair<int, int>(-1, -1);
    }
  }

  printf("Relocation pairs: %d\n", (int)m_RelocPairs.size());

  std::vector<RelocationTableDef>& tables = m_RelocationTables;
  MemAccessMap* map       = emu.debugger().memAccessMap();
  int tblStart            = 0;
  int tblSize             = 0;
  int tblDelta            = 0;
  int tblInc              = 0;
  int tblSizeMax          = 0;
  for (auto it = m_RelocPairs.begin(); it != m_RelocPairs.end(); ++it) {
    if (tblStart == 0) {
      tblStart    = it->first;
      tblDelta    = it->second - it->first;
      tblSize     = 1;
      tblInc      = (tblDelta == 1) ? 2 : 1;
      tblSizeMax  = (tblDelta == 1) ? (0x10000 - tblStart) / 2 : abs(tblDelta);
      assert(tblDelta != -1); // big endian???
    }
    else
    {
      if (it->second - it->first == tblDelta)
      {
        if (it->first == tblStart + tblSize * tblInc) {
          ++tblSize;
          continue;
        }
      }
      bool cont = false;
      if (bridgeGaps) {
        for (int i = tblSize; i < tblSizeMax; ++i) {
          if (it->first == tblStart + tblSize * tblInc && it->second == tblStart + tblDelta + tblSize * tblInc) {
            cont = true;
            break;
          }
#if 0 // TOO DANGEROUS?
          int lo = tblStart + i * tblInc;
          int hi = tblStart + tblDelta + i * tblInc;
          if (map && map->getAccessType(lo) == MemAccessMap::UNTOUCHED && map->getAccessType(hi) == MemAccessMap::UNTOUCHED) {
            // We can't make assumptions about data that has actually been accessed and not tagged as relocatable address
            int addr = emu.getByte(lo) | (emu.getByte(hi) << 8);
            if (addr >= relocRangeStart && addr < relocRangeStart + relocRangeSize) {
              // Address is in relocation range
              ++tblSize;
            } else {
              break;
            }
          } else {
            break;
          }
#else    
          break;
#endif
        }
      }
      if (!cont) {
        auto tbl = RelocationTableDef(tblStart, tblStart + tblDelta, tblSize);
        tables.push_back(tbl);
        tblStart = 0;
      }
      --it;
    }
  }
  if (tblStart) {
    auto tbl = RelocationTableDef(tblStart, tblStart + tblDelta, tblSize);
    tables.push_back(tbl);
  }
  int erased = 0;
  for (auto it = tables.begin(); it != tables.end(); ) {
    int distance = it->m_HiBytes - it->m_LoBytes == 1 ? 2 : 1;
    bool isOk = true;
    for (int i = 0; i < it->m_Size; ++i) {
      int eff = emu.getByte(it->m_LoBytes + i * distance) | (emu.getByte(it->m_HiBytes + i * distance) << 8);
      if (eff < relocRangeStart || eff >= relocRangeStart + relocRangeSize) {
        isOk = false;
        break;
      }
    }
    if (isOk) {
      ++it;
    } else {
      it = tables.erase(it);
      ++erased;
    }
  }
  if (m_DumpRelocationTables) {
    // Print table info
    for (auto it = tables.begin(); it != tables.end(); ++it) {
      int distance = it->m_HiBytes - it->m_LoBytes == 1 ? 2 : 1;
      printf("Tbl: Lo: %04x, Hi: %04x, size %04x ", it->m_LoBytes, it->m_HiBytes, it->m_Size);
      printf("[ ");
      for (int i = 0; i < it->m_Size; ++i) {
        if (i >= 8) {
          printf(" ...");
          break;
        }
        if (i > 0)
          printf(", ");
        int eff = emu.getByte(it->m_LoBytes + i * distance) | (emu.getByte(it->m_HiBytes + i * distance) << 8);
        printf("%04x", eff);
      }
      printf(" ]\n");
    }
    printf("Erased: %d\n", erased);
  }
}

int OperandTraceState::removeOverlappingRelocationTables(int lobytes, int hibytes, int count) {
  int removed = 0;
  for (auto it = m_RelocationTables.begin(); it != m_RelocationTables.end(); ) {
    if (lobytes == it->m_LoBytes && hibytes == it->m_HiBytes) {
      int delta = it->m_Size - count;
      it = m_RelocationTables.erase(it);
      ++removed;
    } else {
      ++it;
    }
  }
  return removed;
}

int OperandTraceState::removeOverlappingRelocationTables(Disassembler::AddressTable* tbl) {
  assert(tbl);
  return removeOverlappingRelocationTables(tbl->m_LoAddr, tbl->m_HiAddr, tbl->m_Size);
}

void OperandTraceState::enableOperandTracing(OperandTracerEmu& emu, int relocRangeStart, int relocRangeSize) {
  emu.debugger().registerTraceExecuteEvaluator(new ExecuteTracer2(emu, this, relocRangeStart, relocRangeSize), 0, 65536, true);
}

void OperandTraceState::init() {
  m_DumpRelocationTables = false;
  m_CurrentTraces = new TraceNodePtr [65536];
  m_A = TraceNode::getUnresolvable();
  m_X = TraceNode::getUnresolvable();
  m_Y = TraceNode::getUnresolvable();
  //assert(TraceNode::s_Unresolvable.refCount() == 4);
}

int TraceNode::leafNodes() const {
  if (m_LeafNodes == 0) {
    int left = m_Left.get()   ? m_Left.get()->leafNodes()  : 1;
    int right = m_Right.get() ? m_Right.get()->leafNodes() : 0;
    m_LeafNodes = left + right;
  }
  return m_LeafNodes;
}

void OperandTraceState::cleanup() {
  m_Unresolvable.clear();
  if (m_CurrentTraces) {
    for (int addr = 0; addr < 65536; ++addr) {
      //auto n = m_CurrentTraces[addr].get();
      //if (n) {
      //  printf("%04x: depth: %3d, leaf nodes: %3d\n", addr, n->maxdepth(), n->leafNodes());
      //}
      m_CurrentTraces[addr].clear();
    }
  }
  m_A.clear();
  m_X.clear();
  m_Y.clear();
  //TraceNode::s_Unresolvable.clear();
  //TraceNode::s_Null.clear();
  delete [] m_CurrentTraces;
  m_CurrentTraces = NULL;
}

}
