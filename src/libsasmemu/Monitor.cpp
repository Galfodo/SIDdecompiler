
#include "Monitor.h"
#include "Util.h"
#include "OpcodeDefs.h"

#include <ctype.h>
#include <stdio.h>
#include <algorithm>

namespace SASM {

// Parameter format strings:
// A - 16-bit address
// E - 16-bit end address. Must be preceded by A. Parameter validation checks that A <= E
// B - 8-bit value
// S - Quoted string
// | - The following parameters are optional
// * - Wildcard. Must be preceded by parameter type (A,E,B,S). 
// + - Repeat. Must be preceded by parameter type (A,E,B,S). 
// ? - Skip validation from this point on
Hue::Util::String Monitor::describeParams(const char* params) {
  Hue::Util::String sResult;
  bool optional = false;
  int index = 0;
  while (params && *params) {
    if (*params == '|') {
      optional = true;
      ++params;
      continue;
    }
    if (!sResult.empty()) {
      sResult.append(" ");
    }
    if (*params == 'A' || *params == 'E') {
      sResult.append(optional ? "[" : "");
      sResult.appendf("%04x", index);
      sResult.append(optional ? "]" : "");
      index += 0x1111;
    } else if (*params == 'B') {
      sResult.append(optional ? "[" : "");
      sResult.appendf("%02x", index & 0xff);
      sResult.append(optional ? "]" : "");
      index += 0x1111;
    } else if (*params == 'S') {
      sResult.append(optional ? "[" : "");
      sResult.append("\"string\"");
      sResult.append(optional ? "]" : "");
    }
    ++params;
    if (*params == '*') {
      sResult.append(" or *");
      ++params;
      assert(*params == '\0');
    } else if (*params == '+') {
      sResult.append(" ...");
      ++params;
      assert(*params == '\0');
    }
  }
  return sResult;
}

bool Monitor::validateParams(const char* params, std::vector<Arg> const& args) {
  int param = 0;
  bool optional = false;
  enum {
    NONE,
    WORD,
    BYTE,
    STRING
  } lastParam = NONE;
  int prevAddress = -1;
  while (params && *params) {
    if (*params == '?') {
      return true;
    }
    if (*params == '|') {
      optional = true;
      ++params;
      continue;
    }
    if (param >= args.size()) {
      if (optional || *params == '*') {
        return true;
      } else {
        writeLine("Not enough parameters for command");
        return false;
      }
    } else {
      Arg const& arg = args[param];
      if (params[1] == '*' && arg.m_Arg == "*") {
        ++param;
        break;
      }
      if (*params == 'A' || *params == 'E') {
        if (!arg.isHex() || arg.hexValue() >= 65536) {
          writeLine("Illegal parameter for command");
          return false;
        }
        if (*params == 'A') {
          prevAddress = arg.hexValue();
        } else {
          assert(prevAddress >= 0);
          if (arg.hexValue() < prevAddress) {
            writeLine("Invalid address range");
            return false;
          }
        }
      } else if (*params == 'B') {
        if (!arg.isHex() || arg.hexValue() >= 256) {
          writeLine("Illegal parameter for command");
          return false;
        }
      } else if (*params == 'S') {
        if (!arg.isString()) {
          writeLine("Illegal parameter for command");
          return false;
        }
      }
      ++params;
      if (*params == '+') {
        --params; // repeat last parameter type
        optional = true;
      }
      ++param;
    }
  }
  if (param < args.size()) {
    writeLine("Too many parameters for command");
    return false;
  }
  return true;
}

struct RecordExecutionHandler : public TraceEvaluator {
  Monitor& m_Monitor;
  RecordExecutionHandler(Monitor& mon) : m_Monitor(mon) {
  }

  int eval(C64MachineState& emu, int addr) override {
    if (m_Monitor.m_HistorySize) {
      int i = m_Monitor.m_HistoryCurrent;
      m_Monitor.m_HistoryBuf[i].m_CPUState = emu.getCPUState();
      m_Monitor.m_HistoryCurrent = (i + 1) & (m_Monitor.m_HistorySize - 1);
      if (m_Monitor.m_HistoryItems < m_Monitor.m_HistorySize) {
        ++m_Monitor.m_HistoryItems;
      }
    }
    return TraceEvaluator::CONTINUE;
  }
};

Monitor::Monitor(C64MachineState& emu) : m_Emu(emu), m_GetLine(NULL), m_Write(NULL), m_Quit(false) {
  setGetLineDelegateT(this, &Monitor::defaultGetLine);
  setWriteDelegateT(this, &Monitor::defaultWrite);
  m_Emu.debugger().registerTraceExecuteEvaluator(new RecordExecutionHandler(*this), 0x0000, 0xffff, true);
  m_HistorySize       = 16;
  m_HistoryBuf        = new HistoryItem[m_HistorySize];
  m_HistoryCurrent    = 0;
  m_HistoryItems      = 0;
  m_PageSize          = 16;
  m_DisasmPC          = 0;
  m_NextBreakpointID  = 1;
  m_BreakpointHit     = 0;
  m_Prompt            = ">";
  m_Banner            = "[ SASMemu Monitor ]\n\nType ? for help\n";

  setVariable("PC", (word*)&m_Emu.PC);
  setVariable("A",  &m_Emu.A);
  setVariable("X",  &m_Emu.X);
  setVariable("Y",  &m_Emu.Y);
  setVariable("SR", &m_Emu.SR);
  setVariable("SP", &m_Emu.SP);

  addCommandT("?",    this, &Monitor::cmdHelp,              "show available commands",                          NULL);
  addCommandT("r",    this, &Monitor::cmdRegs,              "print registers",                                  NULL);
  addCommandT("x",    this, &Monitor::cmdExit,              "resume emulation",                                 NULL);
  addCommandT("q",    this, &Monitor::cmdQuit,              "quit",                                             NULL);
  addCommandT("d",    this, &Monitor::cmdDisassemble,       "disassemble",                                      "|AE");
  addCommandT("m",    this, &Monitor::cmdMemory,            "memory",                                           "|AE");
  addCommandT("b",    this, &Monitor::cmdAddBreakpoint,     "add execution breakpoint",                         "A|E");
  addCommandT("br",   this, &Monitor::cmdAddBreakOnRead,    "add break on read",                                "A|E");
  addCommandT("bw",   this, &Monitor::cmdAddBreakOnWrite,   "add break on write",                               "A|E");
  addCommandT("bl",   this, &Monitor::cmdListBreakpoints,   "list breakpoints",                                 NULL);
  addCommandT("e",    this, &Monitor::cmdRemoveBreakpoint,  "erase breakpoint",                                 "B");
  addCommandT("z",    this, &Monitor::cmdSingleStep,        "step into (call-address A X Y)",                   "|ABBB");
  addCommandT("n",    this, &Monitor::cmdStepOver,          "step over",                                        NULL);
  addCommandT("j",    this, &Monitor::cmdJMP,               "call (call-address repeatcount A X Y)",            "|AABBB");
  addCommandT("hist", this, &Monitor::cmdHistory,           "show cpu history",                                 NULL);
  addCommandT("f",    this, &Monitor::cmdFill,              "fill memory",                                      "AEB+");
  addCommandT("h",    this, &Monitor::cmdHunt,              "search memory",                                    "AEB+");
  addCommandT("a",    this, &Monitor::cmdAssemble,          "assemble instruction",                             "|A?");
  addCommandT("set",  this, &Monitor::cmdSet,               "set variable or print variable(s) (name<=value>)", "?");
}

Monitor::~Monitor() {
  // clean up
}

Monitor::Variable::Variable(const char* name, int value, int size) : m_Name(name), m_Value(value), m_ValuePtr(&m_Value), m_Size(size) {
  assert(m_Size == 1 || m_Size == 2 || m_Size == sizeof(int));
}

Monitor::Variable::Variable(const char* name, void* valuePtr, int size) : m_Name(name), m_Value(0), m_ValuePtr(valuePtr), m_Size(size) {
  assert(m_Size == 1 || m_Size == 2 || m_Size == sizeof(int));
}

int Monitor::Variable::getValue() const {
  switch (m_Size) {
  case 1:
    return *(byte*)m_ValuePtr;
  case 2:
    return *(word*)m_ValuePtr;
  default:
    return *(int*)m_ValuePtr;
  }
}

void Monitor::Variable::setValue(int value) {
  switch (m_Size) {
  case 1:
    *(byte*)m_ValuePtr = value & 0xff;
    break;
  case 2:
    *(word*)m_ValuePtr = value & 0xffff;
    break;
  default:
    assert(m_Size == sizeof(int));
    *(int*)m_ValuePtr = value;
    break;
  }
}

Hue::Util::String Monitor::Variable::toString() const {
  int val = getValue();
  switch(m_Size) {
  case 1:
    return Hue::Util::String::static_printf("%02x", val);
  case 2:
    return Hue::Util::String::static_printf("%04x", val);
  default:
    return Hue::Util::String::static_printf("%08X", val);
  }
}

Monitor::Variable* Monitor::setVariable(const char* nam, byte* valuePtr) {
  Hue::Util::String name(nam); name.toupper();
  auto var = getVariable(name);
  if (var) {
    var->setValue(*valuePtr);
  } else {
    var = new Variable(name, valuePtr, sizeof(*valuePtr));
    m_VariableMap.insert(std::make_pair(name, var));
  }
  return var;
}

Monitor::Variable* Monitor::setVariable(const char* nam, word* valuePtr) {
  Hue::Util::String name(nam); name.toupper();
  auto var = getVariable(name);
  if (var) {
    var->setValue(*valuePtr);
  } else {
    var = new Variable(name, valuePtr, sizeof(*valuePtr));
    m_VariableMap.insert(std::make_pair(name, var));
  }
  return var;
}

Monitor::Variable* Monitor::setVariable(const char* nam, int* valuePtr) {
  Hue::Util::String name(nam); name.toupper();
  auto var = getVariable(name);
  if (var) {
    var->setValue(*valuePtr);
  } else {
    var = new Variable(name, valuePtr, sizeof(*valuePtr));
    m_VariableMap.insert(std::make_pair(name, var));
  }
  return var;
}

Monitor::Variable* Monitor::setVariable(const char* nam, int value) {
  Hue::Util::String name(nam); name.toupper();
  auto var = getVariable(name);
  if (var) {
    var->setValue(value);
  } else {
    var = new Variable(name, value, sizeof(int));
    m_VariableMap.insert(std::make_pair(name, var));
  }
  return var;
}

Monitor::Variable* Monitor::getVariable(const char* nam) {
  Hue::Util::String name(nam); name.toupper();
  auto it = m_VariableMap.find(name);
  if (it != m_VariableMap.end()) {
    return it->second;
  } else {
    return NULL;
  }
}

void Monitor::deleteVariable(const char* nam) {
  Hue::Util::String name(nam); name.toupper();
  auto it = m_VariableMap.find(name);
  if (it != m_VariableMap.end()) {
    delete it->second;
    m_VariableMap.erase(it);
  }
}

int Monitor::Arg::hexValue() const {
  int value = -1;
  sscanf(m_Arg.c_str(), "%x", (unsigned int*)&value);
  return value;
}

bool Monitor::Arg::isHex() const {
  if (m_Arg.empty()) {
    return false;
  }
  for (int i = 0; i < m_Arg.length(); ++i) {
    if (!isxdigit((unsigned char)m_Arg[i])) {
      return false;
    }
  }
  return true;
}

bool Monitor::Arg::isString() const {
  if (m_Arg.length() >= 2 && m_Arg.starts_with("\"") && m_Arg.ends_with("\"")) {
    return true;
  } else {
    return false;
  }
}

Hue::Util::String Monitor::Arg::unquoted() const {
  auto str = m_Arg;
  str.replace("\"", "");
  return str;
}

void Monitor::addCommand(const char* name, CommandDelegate* d, const char* description, const char* args) {
  assert(name && *name);
  assert(d);
  auto cmd = new Command(name, d, description, args);
  m_CommandMap.insert(std::make_pair(name, cmd));
}

void Monitor::setGetLineDelegate(GetLineDelegate* d) {
  assert(d);
  delete m_GetLine;
  m_GetLine = d;
}

void Monitor::setWriteDelegate(WriteDelegate* d) {
  assert(d);
  delete m_Write;
  m_Write = d;
}

void Monitor::defaultWrite(Hue::Util::String const& str) {
  printf("%s\n", str.c_str());
}

Hue::Util::String Monitor::defaultGetLine(const char* prompt) {
  if (prompt) {
    printf("%s", prompt);
  }
  Hue::Util::String result;
  char buf[1024];
  if (fgets(buf, sizeof(buf), stdin)) {
    result = buf;
  }
  return result;
}

void Monitor::writeLine(const char* str) {
  Hue::Util::String s(str);
  writeLine(s);
}

void Monitor::writeLine(Hue::Util::String const& str) {
  m_Write->invoke(str);
}

bool Monitor::tokenize(Hue::Util::String& cmd, std::vector<Arg>& args, const char* str) {
  cmd.clear();
  args.clear();
  Hue::Util::String::List tokens;
  Hue::Util::String token;
  while (*str) {
    while(isspace((unsigned char)*str)) {
      ++str;
    }
    if (*str == '\0')
      break;
    if (*str == '"') {
      if (!token.empty()) {
        tokens.push_back(token);
        token.clear();
      }
      token.append(*str);
      ++str;
      while (*str != '"') {
        if (*str == '\0') {
          writeLine("ERROR: Unterminated string token");
          return false;
        }
        token.append(*str);
        ++str;
      }
      token.append(*str);
      ++str;
    } else if (*str == '=') {
      if (!token.empty()) {
        tokens.push_back(token);
        token.clear();
      }
      token.append(*str);
      tokens.push_back(token);
      token.clear();
      ++str;
    } else {
      token.append(*str);
      ++str;
    }
    if (isspace((unsigned char)*str)) {
      if (!token.empty()) {
        tokens.append(token);
        token.clear();
      }
    }
  }
  if (!token.empty()) {
    tokens.push_back(token);
  }
  if (tokens.size() > 0) {
    cmd = tokens[0];
    for (int i = 1; i < tokens.size(); ++i) {
      args.push_back(Arg(tokens[i]));
    }
    return true;
  } else {
    return false;
  }
}

int Monitor::executeString(const char* str) {
  Hue::Util::String cmd;
  std::vector<Arg> args;
  if (tokenize(cmd, args, str)) {
    auto it = m_CommandMap.find(cmd);
    if (it != m_CommandMap.end()) {
      if (validateParams(it->second->m_Args, args)) {
        int ret = it->second->m_Delegate->invoke(args);
        return 1;
      } else {
        return 0;
      }
    } else {
      writeLine(Hue::Util::String::static_printf("%s?", cmd.c_str()));
      return 0;
    }
  }
  return 0;
}

int Monitor::executeString(Hue::Util::String const& str) {
  return executeString(str.c_str());
}

int Monitor::executeScript(const char* filename) {
  assert(filename);
  Hue::Util::String::List lines;
  if (lines.loadLines(filename))
  {
    for (int i = 0; i < lines.size(); ++i) {
      auto l = lines.at(i);
      m_CommandQueue.append(l);
    }
    return 0;
  }
  else
  {
    writeLine(Hue::Util::String::static_printf("Failed to read monitor command file '%s'", filename).c_str());
    return 1;
  }
}

int Monitor::disasm(int& pc) {
  writeLine(Util::formatOpcode(&m_Emu.m_Mem[pc], pc, true, true, true));
  pc += OpcodeDefs::getOperandSize(m_Emu.m_Mem[pc]) + 1;
  return 0;
}

bool Monitor::stepEmu() {
  if (m_Emu.runCPU() == 0) {
    if (handleBreakpointHit()) {
      return false;
    } else {
      m_DisasmPC = m_Emu.PC;
      writeLine("Stack underflow\n");
      disasm(m_DisasmPC);
      return false;
    }
  } else {
    return true;
  }
}

int Monitor::interactive() {
  m_DisasmPC = m_Emu.PC;
  while (m_CommandQueue.size() > 0) {
    auto cmd = m_CommandQueue.at(0);
    auto echo = cmd;
    echo.prepend(m_Prompt);
    writeLine(echo);
    if (executeString(cmd) == 0) {
      m_CommandQueue.clear();
    } else {
      m_PrevCommand = cmd;
      m_CommandQueue.remove(0);
    }
  }
  writeLine(m_Banner);
  if (!handleBreakpointHit()) {
    executeString("r");
  }
  m_Banner.clear();
  m_StopInteractive = false;
  while (!m_StopInteractive && !m_Quit) {
    Hue::Util::String command = m_GetLine->invoke(m_Prompt.c_str()).trim();
    if (command.empty()) {
      command = m_PrevCommand;
    }
    if (!command.empty()) {
      executeString(command);
      m_PrevCommand = command;
    }
  }
  if (m_Quit) {
    return -1;
  }
  return 0;
}

// BREAKPOINTS //////////////////////////////////////////////////////////////

struct BreakPointEvaluator : public TraceEvaluator {
  int                         m_ID;
  Monitor&                    m_Monitor;
  bool                        m_IsEnabled;
  bool                        m_TempDisabled;
  C64MachineState::Snapshot*  m_Snapshot;

  BreakPointEvaluator(Monitor& mon, int id, bool isEnabled) : m_Monitor(mon), m_ID(id), m_IsEnabled(isEnabled), m_TempDisabled(false), m_Snapshot(NULL) {
  }

  ~BreakPointEvaluator() {
    delete m_Snapshot;
  }

  int eval(C64MachineState& emu, int addr) {
    if (m_TempDisabled) {
      m_TempDisabled = false;
    } else if (m_IsEnabled) {
      m_Snapshot = emu.getSnapshot();
      m_Monitor.m_BreakpointHit = m_ID;
      m_TempDisabled = true;
      return TraceEvaluator::STOP;
    }
    return TraceEvaluator::CONTINUE;
  }
};

Monitor::Breakpoint::Breakpoint(int id, int beginaddr, int endaddr, Type type, BreakPointEvaluator* e) : m_ID(id), m_BeginAddress(beginaddr), m_EndAddress(endaddr), m_Type(type), m_Enabled(true), m_Evaluator(e) {
}

Monitor::Breakpoint::~Breakpoint() {
  delete m_Evaluator;
}

bool Monitor::handleBreakpointHit() {
  if (m_BreakpointHit) {
    writeLine(Hue::Util::String::static_printf("Breakpoint #%02x hit\n", m_BreakpointHit).c_str());
    auto it = m_BreakpointMap.find(m_BreakpointHit);
    assert(it != m_BreakpointMap.end());
    m_Emu.restoreSnapshot(it->second->m_Evaluator->m_Snapshot);
    delete it->second->m_Evaluator->m_Snapshot;
    it->second->m_Evaluator->m_Snapshot = NULL;
    executeString("r");
    m_DisasmPC = m_Emu.PC;
    disasm(m_DisasmPC);
    m_DisasmPC += OpcodeDefs::getOperandSize(m_Emu.m_Mem[m_DisasmPC]) + 1;
    m_Emu.debugger().clearTrap();
    m_BreakpointHit = 0;
    return true;
  } else {
    return false;
  }
}

bool Monitor::validateRangeInclusive(int begin, int end) {
  if (end < begin) {
    writeLine("ERROR: Illegal range");
    return false;
  } else {
    return true;
  }
}

int Monitor::createBreakpoint(std::vector<Arg> const& args, Breakpoint::Type type) {
  assert(args.size() > 0);
  int beginaddr = args[0].hexValue();
  int endaddr = beginaddr;
  if (args.size() > 1) {
    endaddr = args[1].hexValue();
  }
  if (validateRangeInclusive(beginaddr, endaddr)) {
    auto e = new BreakPointEvaluator(*this, m_NextBreakpointID, true);
    auto b = new Breakpoint(m_NextBreakpointID, beginaddr, endaddr, type, e);
    switch (type) {
    case Breakpoint::CODE:
      m_Emu.debugger().registerTraceExecuteEvaluator(e, beginaddr, endaddr - beginaddr + 1, false);
      break;
    case Breakpoint::READ:
      m_Emu.debugger().registerTraceReadEvaluator(e, beginaddr, endaddr - beginaddr + 1, false);
      break;
    case Breakpoint::WRITE:
      m_Emu.debugger().registerTraceWriteEvaluator(e, beginaddr, endaddr - beginaddr + 1, false);
      break;
    }
    m_BreakpointMap.insert(std::make_pair(m_NextBreakpointID, b));
    writeLine(Hue::Util::String::static_printf("Breakpoint #%02x created.", m_NextBreakpointID).c_str());
    ++m_NextBreakpointID;
    return 0;
  } else {
    return -1;
  }
}

int Monitor::deleteBreakpoint(int id) {
  auto it = m_BreakpointMap.find(id);
  if (it != m_BreakpointMap.end()) {
    auto bp = it->second;
    switch (bp->m_Type) {
    case Breakpoint::CODE:
      m_Emu.debugger().removeTraceExecuteEvaluator(bp->m_Evaluator, bp->m_BeginAddress, bp->m_EndAddress - bp->m_BeginAddress + 1);
      break;
    case Breakpoint::READ:
      m_Emu.debugger().removeTraceReadEvaluator(bp->m_Evaluator, bp->m_BeginAddress, bp->m_EndAddress - bp->m_BeginAddress + 1);
      break;
    case Breakpoint::WRITE:
      m_Emu.debugger().removeTraceWriteEvaluator(bp->m_Evaluator, bp->m_BeginAddress, bp->m_EndAddress - bp->m_BeginAddress + 1);
      break;
    }
    m_BreakpointMap.erase(it);
    delete bp;
  }
  return 0;
}

bool Monitor::breakpointPending() {
  return m_BreakpointHit != 0;
}

// COMMANDS /////////////////////////////////////////////////////////////////

int Monitor::cmdExit(std::vector<Arg> const&) {
  m_StopInteractive = true;
  return 0;
}

int Monitor::cmdRegs(std::vector<Arg> const&) {
  writeLine(m_Emu.debugger().printRegs());
  return 0;
}

int Monitor::cmdQuit(std::vector<Arg> const&) {
  m_Quit = true;
  return 0;
}

int Monitor::cmdTest(std::vector<Arg> const& args) {
  writeLine("TEST");
  for (int i = 0; i < (int)args.size(); ++i) {
    writeLine(args.at(i).m_Arg);
  }
  return 0;
}

int Monitor::cmdDisassemble(std::vector<Arg> const& args) {
  int lines = m_PageSize;
  int endPC = 0;
  if (!args.empty()) {
    m_DisasmPC = args[0].hexValue();
    if (args.size() > 1) {
      endPC = args[1].hexValue();
      lines = 0xffff;
    }
  }
  if (endPC == 0) {
    endPC = 0xffff;
  }
  for (; lines > 0 && m_DisasmPC < endPC; --lines) {
    disasm(m_DisasmPC);
  }
  return 0;
}

int Monitor::cmdMemory(std::vector<Arg> const& args) {
  int endPC = 0;
  if (!args.empty()) {
    m_DisasmPC = args[0].hexValue();
    if (args.size() > 1) {
      endPC = args[1].hexValue();
    }
  }
  if (endPC == 0) {
    endPC = m_DisasmPC + m_PageSize * 16;
  }
  for (int i = 0; m_DisasmPC < endPC; ++i) {
    writeLine(Util::formatMem(&m_Emu.m_Mem[m_DisasmPC], m_DisasmPC, 16));
    m_DisasmPC += 16;
  }
  return 0;
}

int Monitor::cmdHelp(std::vector<Arg> const&) {
  Hue::Util::String sText;
  sText.printf("SASMemu commands:\n\n");
  Hue::Util::String::List
    commands,
    params,
    descriptions;

  int maxcommandsize  = 0;
  int maxparamsize    = 0;
  int maxdescsize     = 0;
  for (auto it = m_CommandMap.begin(); it != m_CommandMap.end(); ++it) {
    commands.push_back(it->second->m_Name);
    maxcommandsize = std::max(maxcommandsize, commands.last().length());
    descriptions.push_back(it->second->m_Description);
    maxdescsize = std::max(maxdescsize, descriptions.last().length());
    params.push_back(describeParams(it->second->m_Args));
    maxparamsize = std::max(maxparamsize, params.last().length());
  }
  for (int i = 0; i < commands.size(); ++i) {
    sText.append(commands[i].left_justify(maxcommandsize + 2));
    sText.append(params[i].left_justify(maxparamsize + 2));
    sText.append("; ");
    sText.append(descriptions[i]);
    sText.append("\n");
  }
  writeLine(sText);
  return 0;
}

int Monitor::cmdAddBreakpoint(std::vector<Arg> const& args) {
  createBreakpoint(args, Breakpoint::Type::CODE);
  return 0;
}

int Monitor::cmdAddBreakOnRead(std::vector<Arg> const& args) {
  createBreakpoint(args, Breakpoint::Type::READ);
  return 0;
}

int Monitor::cmdAddBreakOnWrite(std::vector<Arg> const& args) {
  createBreakpoint(args, Breakpoint::Type::WRITE);
  return 0;
}

int Monitor::cmdListBreakpoints(std::vector<Arg> const&) {
  writeLine("Breakpoints:");
  for (auto it = m_BreakpointMap.begin(); it != m_BreakpointMap.end(); ++it) {
    auto bp = it->second;
    Hue::Util::String sText;
    sText.printf("#%02x : ", bp->m_ID);
    if (bp->m_BeginAddress == bp->m_EndAddress) {
      sText.appendf("%04x     ", bp->m_BeginAddress);
    } else {
      sText.appendf("%04x-%04x", bp->m_BeginAddress, bp->m_EndAddress);
    }
    switch(bp->m_Type) {
    case Breakpoint::CODE:
      sText.append(" CODE ");
      break;
    case Breakpoint::WRITE:
      sText.append(" WRITE");
      break;
    case Breakpoint::READ:
      sText.append(" READ ");
      break;
    }
    sText.appendf(" : %s", bp->m_Enabled ? "Enabled" : "Disabled");
    writeLine(sText);
  }
  if (m_BreakpointMap.empty()) {
    writeLine("[None]");
  }
  return 0;
}

int Monitor::cmdRemoveBreakpoint(std::vector<Arg> const& args) {
  for (auto it = args.begin(); it != args.end(); ++it) {
    if (it->m_Arg.equal("*")) {
      while (!m_BreakpointMap.empty()) {
        deleteBreakpoint(m_BreakpointMap.begin()->first);
      }
    } else {
      int id = it->hexValue();
      deleteBreakpoint(id);
    }
  }
  return 0;
}

int Monitor::cmdSingleStep(std::vector<Arg> const& args) {
  int iArg  = 0;
  int A     = 0;
  int X     = 0;
  int Y     = 0;
  int addr  = m_Emu.PC;
  if (args.size() > iArg) {
    addr = args[iArg].hexValue();
    ++iArg;
    if (args.size() > iArg) {
      A = args[iArg].hexValue();
      ++iArg;
      if (args.size() > iArg) {
        X = args[iArg].hexValue();
        ++iArg;
        if (args.size() > iArg) {
          Y = args[iArg].hexValue();
        }
      }
    }
  }
  if (addr != m_Emu.PC) {
    m_Emu.initCPU(addr, A, X, Y);
  }
  stepEmu();
  m_DisasmPC = m_Emu.PC;
  disasm(m_DisasmPC);
  return 0;
}

int Monitor::cmdStepOver(std::vector<Arg> const&) {
  bool isJSR_or_Branch = false;
  switch (m_Emu.m_Mem[m_Emu.PC]) {
  case 0x20:
    isJSR_or_Branch = true;
    break;
  case 0x10:
  case 0x30:
  case 0x50:
  case 0x70:
  case 0x90:
  case 0xb0:
  case 0xd0:
  case 0xf0:
    if (m_Emu.m_Mem[m_Emu.PC + 1] >= 0x80) { // Only step out of backward branches
      isJSR_or_Branch = true;
    }
  }
  if (isJSR_or_Branch) {
    while (m_Emu.PC != m_DisasmPC && stepEmu()) {
    }
  } else {
    stepEmu();
  }
  m_DisasmPC = m_Emu.PC;
  disasm(m_DisasmPC);
  return 0;
}

int Monitor::cmdJMP(std::vector<Arg> const& args) {
  int iArg  = 0;
  int repeat = 1;
  int A     = m_Emu.A;
  int X     = m_Emu.X;
  int Y     = m_Emu.Y;
  int addr  = m_Emu.PC;
  if (args.size() > iArg) {
    addr = args[0].hexValue();
    ++iArg;
    if (args.size() > iArg) {
      repeat = args[iArg].hexValue();
      ++iArg;
      if (args.size() > iArg) {
        A = args[iArg].hexValue();
        ++iArg;
        if (args.size() > iArg) {
          X = args[iArg].hexValue();
          ++iArg;
          if (args.size() > iArg) {
            Y = args[iArg].hexValue();
          }
        }
      }
    }
  }
  for (int i = 0; i < repeat; ++i) {
    if (addr != m_Emu.PC) {
      m_Emu.initCPU(addr, A, X, Y);
    }
    while (m_Emu.runCPU()) {
    }
    if (m_BreakpointHit) {
      handleBreakpointHit();
      break;
    }
    if (m_Emu.debugger().trapOccurred()) {
      break;
    }
  }
  return 0;
}

Hue::Util::String::List Monitor::getHistory() {
  Hue::Util::String::List result;
  for (int i = 0; i < m_HistoryItems; ++i) {
    C64MachineState::CPUState const& item = m_HistoryBuf[(m_HistoryCurrent - 1 - i) & (m_HistorySize - 1)].m_CPUState;
    int pc = item.PC;
    auto line = Util::formatOpcode(&m_Emu.m_Mem[pc], pc, true, true, true);
    line.left_justify(20);
    line.appendf("A:%02x X:%02x Y:%02x SR:%02x SP:%02x", item.A, item.X, item.Y, item.SR, item.SP);
    result.prepend(line);
  }
  return result;
}

int Monitor::cmdHistory(std::vector<Arg> const& args) {
  Hue::Util::String sText = getHistory().join("\n");
  writeLine(sText);
  return 0;
}

int Monitor::cmdFill(std::vector<Arg> const& args) {
  int start = args[0].hexValue();
  int end   = args[1].hexValue();
  int fillvalues = (int)args.size() - 2;
  for (int i = 0; i + start <= end; ++i) {
    m_Emu.m_Mem[start + i] = args[2 + i % fillvalues].hexValue();
  }
  return 0;
}

int Monitor::cmdHunt(std::vector<Arg> const& args) {
  int start = args[0].hexValue();
  int end   = args[1].hexValue();
  int findvalues = (int)args.size() - 2;
  end -= findvalues + 1;
  std::vector<int> values;
  Hue::Util::String sLine;
  for (int i = 2; i < (int)args.size(); ++i) {
    int val = args[i].hexValue();
    if (val < 0 || val > 0xff) {
      writeLine("Value out of range");
      return 0;
    }
    values.push_back(val & 0xff);
  }
  for (int i = 0; i + start <= end; ++i) {
    bool found = true;
    for (int l = 0; l < findvalues; ++l) {
      if (m_Emu.m_Mem[start + i + l] != values[l]) {
        found = false;
        break;
      }
    }
    if (found) {
      sLine.appendf("%04x ", i + start);
      if (sLine.length() > 70) {
        writeLine(sLine);
        sLine.clear();
      }
    }
  }
  if (sLine.length()) {
    writeLine(sLine);
  }
  return 0;
}

int Monitor::cmdAssemble(std::vector<Arg> const& args) {
  if (args.size() > 0) {
    m_DisasmPC = args[0].hexValue();
  }
  Hue::Util::String asmstring;
  for (int i = 1; i < (int)args.size(); ++i) {
    asmstring.append(args[i].m_Arg);
    asmstring.append(" ");
  }
  while (true) {
    if (!asmstring.empty()) {
      std::vector<byte> data;
      if (Util::assembleSimpleLine(data, asmstring.c_str(), m_DisasmPC) != 0) {
        m_Write->invoke(Util::getLastError());
        break;
      } else {
        for (auto it = data.begin(); it != data.end(); ++it) {
          m_Emu.m_Mem[m_DisasmPC++] = *it;
        }
        asmstring.clear();
      }
    }
    Hue::Util::String sPrompt = Hue::Util::String::static_printf("A%04x ", m_DisasmPC);
    asmstring = m_GetLine->invoke(sPrompt.c_str()).trim();
    if (asmstring.empty()) {
      break;
    }
  }
  return 0;
}

int Monitor::cmdSet(std::vector<Arg> const& args) {
  Hue::Util::String prefix;
  if (args.size() > 0) {
    prefix = args[0].m_Arg;
    prefix.toupper();
  }
  if (args.size() > 1) {
    assert(!prefix.empty());
    if (args[1].m_Arg != "=") {
      writeLine("Syntax error");
      return 0;
    }
    if (args.size() == 3) {
      int value = args[2].hexValue();
      if (value >= 0) {
        setVariable(prefix.c_str(), value);
        return 0;
      }
    }
    writeLine("Syntax error");
  } else {
    for (auto it = m_VariableMap.begin(); it != m_VariableMap.end(); ++it) {
      if (it->second->m_Name.starts_with(prefix)) {
        writeLine(Hue::Util::String::static_printf("%s=%s", it->second->m_Name.c_str(), it->second->toString().c_str()).c_str());
      }
    }
  }
  return 0;
}

} // namespace SASM

