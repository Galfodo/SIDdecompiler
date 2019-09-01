#ifndef SASM_EMU6510_C64MACHINESTATE_H_INCLUDED
#define SASM_EMU6510_C64MACHINESTATE_H_INCLUDED

// SASM compile-time configurable 6502 emulator. Based on code from SIDDump http://csdb.dk/release/?id=152422 
// by Cadaver of Covert Bitops

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <map>
#include <memory>

#include "Types.h"
#include "DebuggerState.h"

#define DEBUG_EMULATOR 0

#if DEBUG_EMULATOR
#include "Util.h"
#endif

namespace SASM {

class Assembler;

template<bool COUNT>      void incrementCPUCyclesT(DebuggerState& db, int count) {}
template<bool TRAP_THIS>  void traceStackUnderflowT(DebuggerState& db, int addr, byte sp) {}
template<bool TRAP_THIS>  void traceStackOverflowT(DebuggerState& db, int addr, byte sp) {}
template<bool TRAP_THIS>  void traceBRKT(DebuggerState& db, int addr) {}
template<bool TRAP_THIS>  void traceReadT(DebuggerState& db, int pc, int addr) {}
template<bool TRAP_THIS>  void traceWriteT(DebuggerState& db, int pc, int addr) {}
template<bool TRAP_THIS>  void traceExecuteT(DebuggerState& db, int addr) {}
template<bool TRAP_THIS>  void traceHaltT(DebuggerState& db, int addr) {}

template<> void incrementCPUCyclesT<true>(DebuggerState& db, int count);
template<> void traceStackUnderflowT<true>(DebuggerState& db, int addr, byte sp);
template<> void traceStackOverflowT<true>(DebuggerState& db, int addr, byte sp);
template<> void traceBRKT<true>(DebuggerState& db, int addr);
template<> void traceReadT<true>(DebuggerState& db, int pc, int addr);
template<> void traceWriteT<true>(DebuggerState& db, int pc, int addr);
template<> void traceExecuteT<true>(DebuggerState& db, int addr);
template<> void traceHaltT<true>(DebuggerState& db, int addr);

template<bool ENABLE_PLA=false, bool COUNT_CYCLES=false, int TRAPS = 0>
struct EmuTraits {
public:
  static void incrementCPUCycles(DebuggerState& db, int count) {
    incrementCPUCyclesT<COUNT_CYCLES>(db, count);
  }

  static void traceStackUnderflow(DebuggerState& db, int addr, byte sp) {
    traceStackUnderflowT<(TRAPS & DebuggerState::TRAP_STACK_UNDERFLOW) != 0>(db, addr, sp);
  }

  static void traceStackOverflow(DebuggerState& db, int addr, byte sp) {
    traceStackOverflowT<(TRAPS & DebuggerState::TRAP_STACK_OVERFLOW) != 0>(db, addr, sp);
  }

  static void traceBRK(DebuggerState& db, int addr) {
    traceBRKT<(TRAPS & DebuggerState::TRAP_BRK) != 0>(db, addr);
  }

  static void traceRead(DebuggerState& db, int pc, int addr) {
    traceReadT<(TRAPS & DebuggerState::TRAP_READ) != 0>(db, pc, addr);
  }

  static void traceWrite(DebuggerState& db, int pc, int addr) {
    traceWriteT<(TRAPS & DebuggerState::TRAP_WRITE) != 0>(db, pc, addr);
  }

  static void traceExecute(DebuggerState& db, int addr) {
    traceExecuteT<(TRAPS & DebuggerState::TRAP_EXECUTE) != 0>(db, addr);
  }

  static void traceHalt(DebuggerState& db, int addr) {
    traceHaltT<(TRAPS & DebuggerState::TRAP_HALT) != 0>(db, addr);
  }

};

class C64MachineState;

class MemoryMappedDevice {
  C64MachineState* m_MachineState;
protected:
  inline C64MachineState& machine() {
    assert(m_MachineState);
    return *m_MachineState;
  }
public:
                MemoryMappedDevice();
  virtual       ~MemoryMappedDevice() = 0;
  virtual void  onAttach(class C64MachineState& machine);
  virtual void  reset() = 0;
  virtual void  update(int delta_cycles) = 0;
};

class C64MachineState {
public:
                          C64MachineState();
  virtual                 ~C64MachineState();
  void                    attach(MemoryMappedDevice* device);
  void                    clearMem();
  void                    softReset();
  void                    initCPU(int addr, byte A = 0, byte X = 0, byte Y = 0);
  void                    clearIRQ();
  void                    setIRQ();
  void                    clearNMI();
  void                    setNMI();
  virtual int             runCPU(void) = 0;
  void                    load(int offset, byte const* data, int count, bool init_cpu);
  void                    loadPRG(byte const* data, int count, bool init_cpu);
  bool                    parseAssemblerAssertions(Assembler& assembler);
  inline DebuggerState&   debugger() { return m_Debugger; }
  inline int              getWord(int offset) { return m_Mem[offset] | (m_Mem[offset + 1] << 8); }
  inline byte             getByte(int offset) { return m_Mem[offset]; }
  inline void             putByte(int offset, byte value) { m_Mem[offset] = value; }
  void                    connectDevice(MemoryMappedDevice* device);

  // MEMORY ACCESSORS: These do not trap
  struct ReadWrite {
    byte&       m_Out;
    byte const& m_In;

    ReadWrite(byte& in_out) : m_Out(in_out), m_In(in_out) {
    }

    // read-modify-write ctor: output address may be different from input
    ReadWrite(byte& out, byte const& in) : m_Out(out), m_In(in) {
    }
  };

  inline byte const& _MEM(int address) const {
    return m_Mem[address];
  }

  inline byte& _MEM(int address) {
    return m_Mem[address];
  }

  inline byte const& LO() const {
    return _MEM(PC);
  }

  inline byte const& HI() const {
    return _MEM(PC+1);
  }

  // GET OPERANDS :
  inline byte IMMEDIATE() const {
    return (LO());
  }

  inline int ABSOLUTE() const {
    return (LO() | (HI() << 8));
  }

  inline int ABSOLUTEX() const {
    return (((LO() | (HI() << 8)) + X) & 0xffff);
  }

  inline int ABSOLUTEY() const {
    return (((LO() | (HI() << 8)) + Y) & 0xffff);
  }

  inline byte ZEROPAGE() const {
    return (LO() & 0xff);
  }

  inline byte ZEROPAGEX() const {
    return ((LO() + X) & 0xff);
  }

  inline byte ZEROPAGEY() const {
    return ((LO() + Y) & 0xff);
  }

  inline int INDIRECTX() const {
    return (_MEM((LO() + X) & 0xff) | (_MEM((LO() + X + 1) & 0xff) << 8));
  }

  inline int INDIRECTY() const {
    return (((_MEM(LO()) | (_MEM((LO() + 1) & 0xff) << 8)) + Y) & 0xffff);
  }

  inline int INDIRECTZP() const {
    return (((_MEM(LO()) | (_MEM((LO() + 1) & 0xff) << 8)) + 0) & 0xffff);
  }

  inline byte FETCH() {
    return _MEM(PC++);
  }

  inline void SETPC(int addr) {
    PC = addr;
  }

  enum CPUFlags {
    FN = 0x80,
    FV = 0x40,
    FU = 0x20, // Unused
    FB = 0x10,
    FD = 0x08,
    FI = 0x04,
    FZ = 0x02,
    FC = 0x01
  };

  struct CPUState {
    byte                A;
    byte                X;
    byte                Y; 
    byte                SP; 
    byte                SR; 
    int                 PC;
    bool                BRK;
    bool                IRQ;
    bool                NMI;
    bool                _NMI;
  };

  struct Snapshot {
    CPUState            m_CPUState;
    byte                m_Mem[65536];
  };

  CPUState              getCPUState() const;
  void                  restoreCPUState(CPUState const& cpustate);
  Snapshot*             getSnapshot() const;
  void                  restoreSnapshot(Snapshot* snapshot);

  byte                  A;
  byte                  X;
  byte                  Y; 
  byte                  SP; 
  byte                  SR; 
  int                   PC;
  bool                  BRK;
  bool                  IRQ;
  bool                  NMI;
  bool                  _NMI;

  byte                  m_Mem[65536];
  mutable DebuggerState m_Debugger;
  std::vector<std::unique_ptr<MemoryMappedDevice> >
                        m_Devices;
protected:
  int                   CurrentInstructionPC;
};

template<typename EMUTRAITS>
class C64MachineStateT : public C64MachineState {
public:
  // DEBUGGER HELPERS ///////////////////

  inline void ADDCYCLES(int count) {
    EMUTRAITS::incrementCPUCycles(m_Debugger, count);
  }

  inline void WRITE(int addr) {
    EMUTRAITS::traceWrite(m_Debugger, PC, addr);
  }

  inline void READ(int addr) const {
    EMUTRAITS::traceRead(m_Debugger, PC, addr);
  }

  inline void CHECKUNDERFLOW() const {
    EMUTRAITS::traceStackUnderflow(m_Debugger, PC-1, SP);
  }

  inline void CHECKOVERFLOW() const {
    EMUTRAITS::traceStackOverflow(m_Debugger, PC-1, SP);
  }

  inline void HALT(int addr) {
    EMUTRAITS::traceHalt(m_Debugger, addr);
  }

  inline void CHECKBREAKPOINTS() {
    EMUTRAITS::traceExecute(m_Debugger, PC);
  }

  inline void NOTIMPLEMENTED(byte opcode, int addr) {
    fprintf(stderr, "Unimplemented opcode: %02x at address $%04x\n", (int)opcode, (int)addr);
  }

  // MEMORY ACCESS HELPERS: These may trap

  inline byte const& RMEM(int addr) const {
    READ(addr);
    return _MEM(addr);
  }

  inline byte& WMEM(int addr) {
    WRITE(addr);
    return _MEM(addr);
  }

  inline ReadWrite RMWMEM(int addr) {
    READ(addr);
    WRITE(addr);
    return ReadWrite(WMEM(addr), RMEM(addr));
  }

  inline void PUSH(byte const& data) {
    WMEM(0x0100 + (SP--)) = data;
    CHECKOVERFLOW();
  }

  inline byte POP() {
    CHECKUNDERFLOW();
    return RMEM(0x0100 + (++SP));
  }

  // Helpers for computing instruction timing:
  inline int EVALPAGECROSSING(int baseaddr, int realaddr) { return ((((baseaddr) ^ (realaddr)) & 0xff00) ? 1 : 0); }
  inline int EVALPAGECROSSING_ABSOLUTEX() { return (EVALPAGECROSSING(ABSOLUTE(), ABSOLUTEX()))  ; } 
  inline int EVALPAGECROSSING_ABSOLUTEY() { return (EVALPAGECROSSING(ABSOLUTE(), ABSOLUTEY()))  ; }
  inline int EVALPAGECROSSING_INDIRECTY() { return (EVALPAGECROSSING(INDIRECTZP(), INDIRECTY())); }

  inline void SETFLAGS(byte const& data) {
    if (!data)                          
      SR = (SR & ~FN) | FZ;         
    else                                  
      SR = (SR & ~(FN|FZ)) | (data & FN);                      
  }

  inline void ASSIGNSETFLAGS(byte& dest, byte const& data) {
    dest = data;                         
    if (!dest)                           
      SR = (SR & ~FN) | FZ;        
    else                                 
      SR = (SR & ~(FN|FZ)) | (dest & FN);                       
  }

  inline void BRANCH() {
    ADDCYCLES(1);
    unsigned temp = FETCH();                                         
    if (temp < 0x80)                                        
    {                                                       
      ADDCYCLES(EVALPAGECROSSING(PC, PC + temp));
      SETPC(PC + temp);                                     
    }                                                       
    else                                                    
    {                                                       
      ADDCYCLES(EVALPAGECROSSING(PC, PC + temp - 0x100)); 
      SETPC(PC + temp - 0x100);                             
    }                                                       
  }

  inline void ADC(byte const& data) {
    unsigned tempval = data;          
    unsigned temp;
    if (SR & FD)                                                      
    {                                                                    
      temp = (A & 0xf) + (tempval & 0xf) + (SR & FC);               
      if (temp > 0x9)                                                  
        temp += 0x6;                                                 
      if (temp <= 0x0f)                                                
        temp = (temp & 0xf) + (A & 0xf0) + (tempval & 0xf0);         
      else                                                             
        temp = (temp & 0xf) + (A & 0xf0) + (tempval & 0xf0) + 0x10;  
      if (!((A + tempval + (SR & FC)) & 0xff))                      
        SR |= FZ;                                                 
      else                                                             
        SR &= ~FZ;                                                
      if (temp & 0x80)                                                 
        SR |= FN;                                                 
      else                                                             
        SR &= ~FN;                                                
      if (((A ^ temp) & 0x80) && !((A ^ tempval) & 0x80))              
        SR |= FV;                                                 
      else                                                             
        SR &= ~FV;                                                
      if ((temp & 0x1f0) > 0x90) temp += 0x60;                         
      if ((temp & 0xff0) > 0xf0)                                       
        SR |= FC;                                                 
      else                                                             
        SR &= ~FC;                                                
    }                                                                    
    else                                                                 
    {                                                                    
      temp = tempval + A + (SR & FC);                               
      SETFLAGS(temp & 0xff);                                           
      if (!((A ^ tempval) & 0x80) && ((A ^ temp) & 0x80))              
        SR |= FV;                                                 
      else                                                             
        SR &= ~FV;                                                
      if (temp > 0xff)                                                 
        SR |= FC;                                                 
      else                                                             
        SR &= ~FC;                                                
    }                                                                    
    A = temp;                                                            
  }

  inline void SBC(byte const& data) {
    unsigned tempval = data;                                             
    unsigned temp = A - tempval - ((SR & FC) ^ FC);                            
    if (SR & FD)                                                      
    {                                                                    
      unsigned tempval2;                                               
      tempval2 = (A & 0xf) - (tempval & 0xf) - ((SR & FC) ^ FC);    
      if (tempval2 & 0x10)                                             
        tempval2 = ((tempval2 - 6) & 0xf) | ((A & 0xf0) - (tempval & 0xf0) - 0x10);                                             
      else                                                             
        tempval2 = (tempval2 & 0xf) | ((A & 0xf0) - (tempval & 0xf0));                                                    
      if (tempval2 & 0x100)                                            
        tempval2 -= 0x60;                                            
      if (temp < 0x100)                                                
        SR |= FC;                                                 
      else                                                             
        SR &= ~FC;                                                
      SETFLAGS(temp & 0xff);                                           
      if (((A ^ temp) & 0x80) && ((A ^ tempval) & 0x80))               
        SR |= FV;                                                 
      else                                                             
        SR &= ~FV;                                                
      A = tempval2;                                                    
    }                                                                    
    else                                                                 
    {                                                                    
      SETFLAGS(temp & 0xff);                                           
      if (temp < 0x100)                                                
        SR |= FC;                                                 
      else                                                             
        SR &= ~FC;                                                
      if (((A ^ temp) & 0x80) && ((A ^ tempval) & 0x80))               
        SR |= FV;                                                 
      else                                                             
        SR &= ~FV;                                                
      A = temp;                                                        
    }                                                                    
  }

  inline void CMP(byte const& src, byte const& data) {
    unsigned temp = (src - data) & 0xff;           
    SR = (SR & ~(FC|FN|FZ)) | (temp & FN);                  
    if (!temp) 
      SR |= FZ;               
    if (src >= data) 
      SR |= FC;         
  }

  inline void ASL(ReadWrite rw) {                                       
    unsigned temp = rw.m_In;                          
    temp <<= 1;                           
    if (temp & 0x100) 
      SR |= FC;        
    else 
      SR &= ~FC;                    
    ASSIGNSETFLAGS(rw.m_Out, temp);           
  }

  inline void LSR(ReadWrite rw) {                                       
    unsigned temp = rw.m_In;                          
    if (temp & 1) 
      SR |= FC;            
    else 
      SR &= ~FC;                    
    temp >>= 1;                           
    ASSIGNSETFLAGS(rw.m_Out, temp);           
  }

  inline void ROL(ReadWrite rw) {                                       
    unsigned temp = rw.m_In;                          
    temp <<= 1;                           
    if (SR & FC) 
      temp |= 1;            
    if (temp & 0x100) 
      SR |= FC;        
    else 
      SR &= ~FC;                    
    ASSIGNSETFLAGS(rw.m_Out, temp);           
  }

  inline void ROR(ReadWrite rw) {                                       
    unsigned temp = rw.m_In;                          
    if (SR & FC) 
      temp |= 0x100;        
    if (temp & 1) 
      SR |= FC;            
    else 
      SR &= ~FC;                    
    temp >>= 1;                           
    ASSIGNSETFLAGS(rw.m_Out, temp);           
  }

  inline void DEC(ReadWrite rw) {                                       
    unsigned temp = rw.m_In;
    --temp;
    ASSIGNSETFLAGS(rw.m_Out, temp);           
  }

  inline void INC(ReadWrite rw) {                                       
    unsigned temp = rw.m_In;
    ++temp;
    ASSIGNSETFLAGS(rw.m_Out, temp);           
  }

  inline void EOR(byte const& data) {                                       
    A ^= data;                            
    SETFLAGS(A);                          
  }

  inline void ORA(byte const& data) {                                       
    A |= data;                            
    SETFLAGS(A);                          
  }

  inline void AND(byte const& data) {                                       
    A &= data;                            
    SETFLAGS(A);
  }

  inline void BIT(byte const& data) {                                       
    SR = (SR & ~(FN|FV)) | (data & (FN|FV));             
    if (!(data & A)) 
      SR |= FZ;         
    else 
      SR &= ~FZ;                    
  }

  virtual int runCPU(void) override
  {
    static const int cpucycles_table[] = 
    {
      0,  6,  0,  8,  3,  3,  5,  5,  3,  2,  2,  2,  4,  4,  6,  6, 
      2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7, 
      6,  6,  0,  8,  3,  3,  5,  5,  4,  2,  2,  2,  4,  4,  6,  6, 
      2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7, 
      6,  6,  0,  8,  3,  3,  5,  5,  3,  2,  2,  2,  3,  4,  6,  6, 
      2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7, 
      6,  6,  0,  8,  3,  3,  5,  5,  4,  2,  2,  2,  5,  4,  6,  6, 
      2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7, 
      2,  6,  2,  6,  3,  3,  3,  3,  2,  2,  2,  2,  4,  4,  4,  4, 
      2,  6,  0,  6,  4,  4,  4,  4,  2,  5,  2,  5,  5,  5,  5,  5, 
      2,  6,  2,  6,  3,  3,  3,  3,  2,  2,  2,  2,  4,  4,  4,  4, 
      2,  5,  0,  5,  4,  4,  4,  4,  2,  4,  2,  4,  4,  4,  4,  4, 
      2,  6,  2,  8,  3,  3,  5,  5,  2,  2,  2,  2,  4,  4,  6,  6, 
      2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7, 
      2,  6,  2,  8,  3,  3,  5,  5,  2,  2,  2,  2,  4,  4,  6,  6, 
      2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7
    };

    unsigned temp = 0;

    CHECKBREAKPOINTS();
    if (m_Debugger.trapOccurred()) {
      return 0;
    }

    if (this->IRQ && !(this->SR & CPUFlags::FI) || // Trigger IRQ only if interrupt disable flag is not set
        this->NMI && !this->_NMI ||                // Trigger NMI only on edge
        this->BRK                                  // BRK instruction occurred
      )
    {
      PUSH((PC) >> 8);
      PUSH((PC) & 0xff);
      PUSH(this->BRK ? (SR | CPUFlags::FB) : (SR &~ CPUFlags::FB));
      BRK = false;
      if (this->NMI) {
        PC = getWord(0xfffa);
      } else {
        PC = getWord(0xfffe);
      }
      SR |= CPUFlags::FI;
      SR &= ~CPUFlags::FD;
      ADDCYCLES(7);
    }

#if DEBUG_EMULATOR
    auto stateString = Util::formatOpcode(&m_Mem[PC], PC, true, true, true);
#endif
    CurrentInstructionPC = PC;
    unsigned char op = FETCH();
    ADDCYCLES(cpucycles_table[op]);
    switch(op)
    {
      case 0xa7:
      ASSIGNSETFLAGS(A, RMEM(ZEROPAGE()));
      X = A;
      PC++;
      break;

      case 0xb7:
      ASSIGNSETFLAGS(A, RMEM(ZEROPAGEY()));
      X = A;
      PC++;
      break;

      case 0xaf:
      ASSIGNSETFLAGS(A, RMEM(ABSOLUTE()));
      X = A;
      PC += 2;
      break;

      case 0xa3:
      ASSIGNSETFLAGS(A, RMEM(INDIRECTX()));
      X = A;
      PC++;
      break;

      case 0xb3:
      ADDCYCLES(EVALPAGECROSSING_INDIRECTY());
      ASSIGNSETFLAGS(A, RMEM(INDIRECTY()));
      X = A;
      PC++;
      break;
    
      case 0x1a:
      case 0x3a:
      case 0x5a:
      case 0x7a:
      case 0xda:
      case 0xfa:
      break;
    
      case 0x80:
      case 0x82:
      case 0x89:
      case 0xc2:
      case 0xe2:
      case 0x04:
      case 0x44:
      case 0x64:
      case 0x14:
      case 0x34:
      case 0x54:
      case 0x74:
      case 0xd4:
      case 0xf4:
      PC++;
      break;
    
      case 0x0c:
      case 0x1c:
      case 0x3c:
      case 0x5c:
      case 0x7c:
      case 0xdc:
      case 0xfc:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEX());
      PC += 2;
      break;

      case 0x69:
      ADC(IMMEDIATE());
      PC++;
      break;

      case 0x65:
      ADC(RMEM(ZEROPAGE()));
      PC++;
      break;

      case 0x75:
      ADC(RMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0x6d:
      ADC(RMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0x7d:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEX());
      ADC(RMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0x79:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEY());
      ADC(RMEM(ABSOLUTEY()));
      PC += 2;
      break;

      case 0x61:
      ADC(RMEM(INDIRECTX()));
      PC++;
      break;

      case 0x71:
      ADDCYCLES(EVALPAGECROSSING_INDIRECTY());
      ADC(RMEM(INDIRECTY()));
      PC++;
      break;

      case 0x29:
      AND(IMMEDIATE());
      PC++;
      break;

      case 0x25:
      AND(RMEM(ZEROPAGE()));
      PC++;
      break;

      case 0x35:
      AND(RMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0x2d:
      AND(RMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0x3d:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEX());
      AND(RMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0x39:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEY());
      AND(RMEM(ABSOLUTEY()));
      PC += 2;
      break;

      case 0x21:
      AND(RMEM(INDIRECTX()));
      PC++;
      break;

      case 0x31:
      ADDCYCLES(EVALPAGECROSSING_INDIRECTY());
      AND(RMEM(INDIRECTY()));
      PC++;
      break;

      case 0x0a:
      ASL(A);
      break;

      case 0x06:
      ASL(RMWMEM(ZEROPAGE()));
      PC++;
      break;

      case 0x16:
      ASL(RMWMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0x0e:
      ASL(RMWMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0x1e:
      ASL(RMWMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0x90:
      if (!(SR & FC)) 
        BRANCH();
      else 
        PC++;
      break;

      case 0xb0:
      if (SR & FC) 
        BRANCH();
      else PC++;
      break;

      case 0xf0:
      if (SR & FZ) 
        BRANCH();
      else PC++;
      break;

      case 0x24:
      BIT(RMEM(ZEROPAGE()));
      PC++;
      break;

      case 0x2c:
      BIT(RMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0x30:
      if (SR & FN) 
        BRANCH();
      else 
        PC++;
      break;

      case 0xd0:
      if (!(SR & FZ)) 
        BRANCH();
      else 
      PC++;
      break;

      case 0x10:
      if (!(SR & FN)) 
        BRANCH();
      else 
        PC++;
      break;

      case 0x50:
      if (!(SR & FV)) 
        BRANCH();
      else 
        PC++;
      break;

      case 0x70:
      if (SR & FV) 
        BRANCH();
      else 
        PC++;
      break;

      case 0x18:
      SR &= ~FC;
      break;

      case 0xd8:
      SR &= ~FD;
      break;

      case 0x58:
      SR &= ~FI;
      break;

      case 0xb8:
      SR &= ~FV;
      break;

      case 0xc9:
      CMP(A, IMMEDIATE());
      PC++;
      break;

      case 0xc5:
      CMP(A, RMEM(ZEROPAGE()));
      PC++;
      break;

      case 0xd5:
      CMP(A, RMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0xcd:
      CMP(A, RMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0xdd:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEX());
      CMP(A, RMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0xd9:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEY());
      CMP(A, RMEM(ABSOLUTEY()));
      PC += 2;
      break;

      case 0xc1:
      CMP(A, RMEM(INDIRECTX()));
      PC++;
      break;

      case 0xd1:
      ADDCYCLES(EVALPAGECROSSING_INDIRECTY());
      CMP(A, RMEM(INDIRECTY()));
      PC++;
      break;

      case 0xe0:
      CMP(X, IMMEDIATE());
      PC++;
      break;

      case 0xe4:
      CMP(X, RMEM(ZEROPAGE()));
      PC++;
      break;

      case 0xec:
      CMP(X, RMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0xc0:
      CMP(Y, IMMEDIATE());
      PC++;
      break;

      case 0xc4:
      CMP(Y, RMEM(ZEROPAGE()));
      PC++;
      break;

      case 0xcc:
      CMP(Y, RMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0xc6:
      DEC(RMWMEM(ZEROPAGE()));
      PC++;
      break;

      case 0xd6:
      DEC(RMWMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0xce:
      DEC(RMWMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0xde:
      DEC(RMWMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0xca:
      X--;
      SETFLAGS(X);
      break;

      case 0x88:
      Y--;
      SETFLAGS(Y);
      break;

      case 0x49:
      EOR(IMMEDIATE());
      PC++;
      break;

      case 0x45:
      EOR(RMEM(ZEROPAGE()));
      PC++;
      break;

      case 0x55:
      EOR(RMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0x4d:
      EOR(RMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0x5d:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEX());
      EOR(RMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0x59:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEY());
      EOR(RMEM(ABSOLUTEY()));
      PC += 2;
      break;

      case 0x41:
      EOR(RMEM(INDIRECTX()));
      PC++;
      break;

      case 0x51:
      ADDCYCLES(EVALPAGECROSSING_INDIRECTY());
      EOR(RMEM(INDIRECTY()));
      PC++;
      break;

      case 0xe6:
      INC(RMWMEM(ZEROPAGE()));
      PC++;
      break;

      case 0xf6:
      INC(RMWMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0xee:
      INC(RMWMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0xfe:
      INC(RMWMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0xe8:
      X++;
      SETFLAGS(X);
      break;

      case 0xc8:
      Y++;
      SETFLAGS(Y);
      break;

      case 0x20:
      PUSH((PC+1) >> 8);
      PUSH((PC+1) & 0xff);
      PC = ABSOLUTE();
      break;

      case 0x4c:
      PC = ABSOLUTE();
      break;

      case 0x6c:
      {
        int adr = ABSOLUTE();
        PC = (RMEM(adr) | (RMEM(((adr + 1) & 0xff) | (adr & 0xff00)) << 8));
      }
      break;

      case 0xa9:
      ASSIGNSETFLAGS(A, IMMEDIATE());
      PC++;
      break;

      case 0xa5:
      ASSIGNSETFLAGS(A, RMEM(ZEROPAGE()));
      PC++;
      break;

      case 0xb5:
      ASSIGNSETFLAGS(A, RMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0xad:
      ASSIGNSETFLAGS(A, RMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0xbd:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEX());
      ASSIGNSETFLAGS(A, RMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0xb9:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEY());
      ASSIGNSETFLAGS(A, RMEM(ABSOLUTEY()));
      PC += 2;
      break;

      case 0xa1:
      ASSIGNSETFLAGS(A, RMEM(INDIRECTX()));
      PC++;
      break;

      case 0xb1:
      ADDCYCLES(EVALPAGECROSSING_INDIRECTY());
      ASSIGNSETFLAGS(A, RMEM(INDIRECTY()));
      PC++;
      break;

      case 0xa2:
      ASSIGNSETFLAGS(X, IMMEDIATE());
      PC++;
      break;

      case 0xa6:
      ASSIGNSETFLAGS(X, RMEM(ZEROPAGE()));
      PC++;
      break;

      case 0xb6:
      ASSIGNSETFLAGS(X, RMEM(ZEROPAGEY()));
      PC++;
      break;

      case 0xae:
      ASSIGNSETFLAGS(X, RMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0xbe:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEY());
      ASSIGNSETFLAGS(X, RMEM(ABSOLUTEY()));
      PC += 2;
      break;

      case 0xa0:
      ASSIGNSETFLAGS(Y, IMMEDIATE());
      PC++;
      break;

      case 0xa4:
      ASSIGNSETFLAGS(Y, RMEM(ZEROPAGE()));
      PC++;
      break;

      case 0xb4:
      ASSIGNSETFLAGS(Y, RMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0xac:
      ASSIGNSETFLAGS(Y, RMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0xbc:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEX());
      ASSIGNSETFLAGS(Y, RMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0x4a:
      LSR(A);
      break;

      case 0x46:
      LSR(RMWMEM(ZEROPAGE()));
      PC++;
      break;

      case 0x56:
      LSR(RMWMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0x4e:
      LSR(RMWMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0x5e:
      LSR(RMWMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0xea:
      break;

      case 0x09:
      ORA(IMMEDIATE());
      PC++;
      break;

      case 0x05:
      ORA(RMEM(ZEROPAGE()));
      PC++;
      break;

      case 0x15:
      ORA(RMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0x0d:
      ORA(RMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0x1d:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEX());
      ORA(RMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0x19:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEY());
      ORA(RMEM(ABSOLUTEY()));
      PC += 2;
      break;

      case 0x01:
      ORA(RMEM(INDIRECTX()));
      PC++;
      break;

      case 0x11:
      ADDCYCLES(EVALPAGECROSSING_INDIRECTY());
      ORA(RMEM(INDIRECTY()));
      PC++;
      break;

      case 0x48:
      PUSH(A);
      break;

      case 0x08:
      PUSH(SR);
      break;

      case 0x68:
      ASSIGNSETFLAGS(A, POP());
      break;

      case 0x28:
      SR = POP() & ~CPUFlags::FB;
      break;

      case 0x2a:
      ROL(A);
      break;

      case 0x26:
      ROL(RMWMEM(ZEROPAGE()));
      PC++;
      break;

      case 0x36:
      ROL(RMWMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0x2e:
      ROL(RMWMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0x3e:
      ROL(RMWMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0x6a:
      ROR(A);
      break;

      case 0x66:
      ROR(RMWMEM(ZEROPAGE()));
      PC++;
      break;

      case 0x76:
      ROR(RMWMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0x6e:
      ROR(RMWMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0x7e:
      ROR(RMWMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0x40:
      if (SP == 0xff) {
        --PC;
        return 0; // FIXME this should use Trap mechanism
      }
      SR = POP() & ~CPUFlags::FB;
      PC = POP();
      PC |= (int)POP() << 8;
      break;

      case 0x60:
      if (SP == 0xff) {
        --PC;
        return 0; // FIXME this should use Trap mechanism
      }
      PC = POP();
      PC |= (int)POP() << 8;
      PC++;
      break;

      case 0xe9:
      SBC(IMMEDIATE());
      PC++;
      break;

      case 0xe5:
      SBC(RMEM(ZEROPAGE()));
      PC++;
      break;

      case 0xf5:
      SBC(RMEM(ZEROPAGEX()));
      PC++;
      break;

      case 0xed:
      SBC(RMEM(ABSOLUTE()));
      PC += 2;
      break;

      case 0xfd:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEX());
      SBC(RMEM(ABSOLUTEX()));
      PC += 2;
      break;

      case 0xf9:
      ADDCYCLES(EVALPAGECROSSING_ABSOLUTEY());
      SBC(RMEM(ABSOLUTEY()));
      PC += 2;
      break;

      case 0xe1:
      SBC(RMEM(INDIRECTX()));
      PC++;
      break;

      case 0xf1:
      ADDCYCLES(EVALPAGECROSSING_INDIRECTY());
      SBC(RMEM(INDIRECTY()));
      PC++;
      break;

      case 0x38:
      SR |= FC;
      break;

      case 0xf8:
      SR |= FD;
      break;

      case 0x78:
      SR |= FI;
      break;

      case 0x85:
      WMEM(ZEROPAGE()) = A;
      PC++;
      break;

      case 0x95:
      WMEM(ZEROPAGEX()) = A;
      PC++;
      break;

      case 0x8d:
      WMEM(ABSOLUTE()) = A;
      PC += 2;
      break;

      case 0x9d:
      WMEM(ABSOLUTEX()) = A;
      PC += 2;
      break;

      case 0x99:
      WMEM(ABSOLUTEY()) = A;
      PC += 2;
      break;

      case 0x81:
      WMEM(INDIRECTX()) = A;
      PC++;
      break;

      case 0x91:
      WMEM(INDIRECTY()) = A;
      PC++;
      break;

      case 0x86:
      WMEM(ZEROPAGE()) = X;
      PC++;
      break;

      case 0x96:
      WMEM(ZEROPAGEY()) = X;
      PC++;
      break;

      case 0x8e:
      WMEM(ABSOLUTE()) = X;
      PC += 2;
      break;

      case 0x84:
      WMEM(ZEROPAGE()) = Y;
      PC++;
      break;

      case 0x94:
      WMEM(ZEROPAGEX()) = Y;
      PC++;
      break;

      case 0x8c:
      WMEM(ABSOLUTE()) = Y;
      PC += 2;
      break;

      case 0xaa:
      ASSIGNSETFLAGS(X, A);
      break;

      case 0xba:
      ASSIGNSETFLAGS(X, SP);
      break;

      case 0x8a:
      ASSIGNSETFLAGS(A, X);
      break;

      case 0x9a:
      ASSIGNSETFLAGS(SP, X);
      break;

      case 0x98:
      ASSIGNSETFLAGS(A, Y);
      break;

      case 0xa8:
      ASSIGNSETFLAGS(Y, A);
      break;

      case 0x00:
      BRK = true;
      ++PC;
      break;

      case 0x02:
      HALT(PC-1);
      return 0;
          
      default:
      NOTIMPLEMENTED(op, PC-1);
      return 0;
    }
#if DEBUG_EMULATOR
    printf("%s  A:%02x X:%02x Y:%02x\n", stateString.c_str(), (int)A, (int)X, (int)Y);
#endif
    CurrentInstructionPC = PC;
    return m_Debugger.trapOccurred() ? 0 : 1;
  }
};

typedef C64MachineStateT<EmuTraits<false, false, DebuggerState::TRAP_NONE> > MinimalEmu; // No C64-PLA, No cycle counting, no traps
typedef C64MachineStateT<EmuTraits<false, true, DebuggerState::TRAP_NONE> > MinimalCycleCountingEmu; // No C64-PLA, cycle counting, no traps
typedef C64MachineStateT<EmuTraits<true, true, DebuggerState::TRAP_ALL> > FullEmu; // C64-PLA, cycle counting, all trap types

}

#endif
