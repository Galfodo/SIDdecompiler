
#include "Profiling.h"
#include "OpcodeDefs.h"
#include <assert.h>

namespace SASM {

MemAccessMap::MemAccessMap(int addr, int size) {
  // Align addr and size with ROW_SIZE
  size += addr & (ROW_SIZE - 1);
  addr = addr & ~(ROW_SIZE - 1);
  size = (size + ROW_SIZE - 1) & ~(ROW_SIZE - 1);
  assert(addr + size <= 65536);
  m_Address         = addr;
  m_Size            = size;
  m_Data            = new byte[size];
  m_LowestAccessed  = 0xffff;
  m_HighestAccessed = 0x0000;
  m_ExcludeIO       = true;
  memset(m_Data, 0, size);
}

MemAccessMap::~MemAccessMap() {
  delete [] m_Data;
  for (auto i = 0; i < m_Managed.size(); ++i) {
    m_Managed.at(i).Destroy();
  }
}

int MemAccessMap::getAccessType(int addr) {
  if (addr >= m_Address && addr < m_Address + m_Size) {
    return m_Data[addr - m_Address];
  }
  return UNTOUCHED;
}

static const char*
  s_digits= "0123456789ABCDEF";

Hue::Util::String MemAccessMap::printMap(int addr, int lastaddr) {
  int size = lastaddr - addr + 1;
  size += addr & (ROW_SIZE - 1);
  addr = addr & ~(ROW_SIZE - 1);
  size = (size + ROW_SIZE - 1) & ~(ROW_SIZE - 1);
  int rows = (int)size / ROW_SIZE;
  int offset = addr - m_Address;
  assert(offset >= 0);
  Hue::Util::String sData;
  for (int i = 0; i < rows; ++i) {
    char row[ROW_SIZE + 1];
    row[ROW_SIZE] = '\0';
    for (int x = 0; x < ROW_SIZE; ++x) {
      auto data = m_Data[offset + i * ROW_SIZE + x];
      char usage;
      switch(data) {
      case UNTOUCHED:
        usage = '?';
        break;
      case READ:
        usage = 'r';
        break;
      case WRITE:
        usage = 'w';
        break;
      case (READ|WRITE):
        usage = '+';
        break;
      case EXECUTE:
        usage = 'x';
        break;
      case (EXECUTE|WRITE):
        usage = '#';
        break;
      case OPERAND:
        usage = 'o';
        break;
      case (OPERAND|WRITE):
        usage = '_';
        break;
      default:
        assert(data < 16);
        usage = s_digits[data];
      }
      row[x] = usage;
    }
    sData.appendf("%$%04x: %s\n", addr + i * ROW_SIZE, row);
  }
  return sData;
}

void MemAccessMap::dump() {
  int rows = (int)m_Size / ROW_SIZE;
  Hue::Util::String sData;
  sData.printf("Emulated memory access map. Start: $%04x End: $%04x\n", m_LowestAccessed, m_HighestAccessed);
  sData.appendf("Map legend:\n");
  sData.appendf("r   : read access only\n");
  sData.appendf("w   : write access only\n");
  sData.appendf("+   : read + write access\n");
  sData.appendf("x   : execute access only\n");
  sData.appendf("#   : execute + write access (self-modifying code)\n");
  sData.appendf("o   : operand (implicit read) access only\n");
  sData.appendf("_   : operand + write access (self modifying code)\n");
  sData.appendf("0-F : other combinations of Read/Write/Execute/Operand\n");
  sData.appendf("?   : never accessed\n");
  sData.append("\n");
  sData.append(printMap(m_LowestAccessed, m_HighestAccessed));
  printf("\n%s\n", sData.c_str());
  m_TraceState.dumpStats();
}

template<int ACCESSTYPE>
struct MemAccessEvaluator : TraceEvaluator {
  MemAccessEvaluator(MemAccessMap* memAccessMap) : m_MemAccessMap(memAccessMap) {
  }

  int eval(C64MachineState& emu, int addr) override {
    if (emu.m_Mem[emu.PC - 1] == 0x2c) {
      //int debug = 0;
    } else {
      m_MemAccessMap->recordAccess(addr, ACCESSTYPE);
    }
    return TraceEvaluator::CONTINUE;
  }

  MemAccessMap* m_MemAccessMap;
};

template<>
struct MemAccessEvaluator<MemAccessMap::EXECUTE> : TraceEvaluator {
  MemAccessEvaluator(MemAccessMap* memAccessMap) : m_MemAccessMap(memAccessMap) {
  }

  int eval(C64MachineState& emu, int addr) override {
    m_MemAccessMap->recordAccess(addr, MemAccessMap::EXECUTE);
    byte op = emu.m_Mem[addr];
    switch (OpcodeDefs::getOperandSize(op)) {
    case 2:
      m_MemAccessMap->recordAccess(addr + 2, MemAccessMap::OPERAND);
    case 1:
      m_MemAccessMap->recordAccess(addr + 1, MemAccessMap::OPERAND);
    }
    return TraceEvaluator::CONTINUE;
  }

  MemAccessMap* m_MemAccessMap;
};


void Profiling::attachMemAccessMap(OperandTracerEmu& emu, MemAccessMap* memAccessMap, int relocationRegionAddress, int relocationRegionSize) {
  assert(memAccessMap);

  emu.debugger().setMemAccessMap(memAccessMap);

  // Enable memory access tracing
  emu.debugger().registerTraceExecuteEvaluator(new MemAccessEvaluator<MemAccessMap::EXECUTE>(memAccessMap), memAccessMap->m_Address, memAccessMap->m_Size, true);
  emu.debugger().registerTraceReadEvaluator(new MemAccessEvaluator<MemAccessMap::READ>(memAccessMap), memAccessMap->m_Address, memAccessMap->m_Size, true);
  emu.debugger().registerTraceWriteEvaluator(new MemAccessEvaluator<MemAccessMap::WRITE>(memAccessMap), memAccessMap->m_Address, memAccessMap->m_Size, true);

  // Enable operand tracing
  memAccessMap->m_TraceState.enableOperandTracing(emu, relocationRegionAddress, relocationRegionSize);
}

} // namespace SASM
