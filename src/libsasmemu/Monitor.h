#ifndef SASM_MONITOR_H_INCLUDED
#define SASM_MONITOR_H_INCLUDED

#include "Types.h"
#include "C64MachineState.h"

#include <map>
#include <vector>

namespace SASM {

struct BreakPointEvaluator;
struct RecordExecutionHandler;

class Monitor {
public:
  struct Arg {
    Hue::Util::String         m_Arg;

                              Arg(Hue::Util::String const& arg) : m_Arg(arg) {
                              }
    int                       hexValue() const;
    bool                      isHex() const;
    bool                      isString() const;
    Hue::Util::String         unquoted() const;
  };

  struct GetLineDelegate {
                              ~GetLineDelegate() {}
    virtual Hue::Util::String invoke(const char*) = 0;
  };

  template<typename T>
  struct GetLineDelegateT : public GetLineDelegate {
    T* m_Instance;
    Hue::Util::String         (T::*m_Callback)(const char*);
                              GetLineDelegateT(T* instance, Hue::Util::String (T::*callback)(const char*)) : m_Instance(instance), m_Callback(callback) { }
    virtual Hue::Util::String invoke(const char* prompt) {
                                return (m_Instance->*m_Callback)(prompt);
                              }
  };

  struct WriteDelegate {
                              ~WriteDelegate() {}
    virtual void              invoke(Hue::Util::String const& str) = 0;
  };

  template<typename T>
  struct WriteDelegateT : public WriteDelegate {
    T*                        m_Instance;
    void                      (T::*m_Callback)(Hue::Util::String const&);
                              WriteDelegateT(T* instance, void (T::*callback)(Hue::Util::String const&)) : m_Instance(instance), m_Callback(callback) { }
    virtual void              invoke(Hue::Util::String const& str) {
                                (m_Instance->*m_Callback)(str);
                              }
  };

  struct CommandDelegate {
                              ~CommandDelegate() {}
    virtual int               invoke(std::vector<Arg> const& argList) = 0;
  };

  template<typename T>
  struct CommandDelegateT : public CommandDelegate {
    T*                        m_Instance;
    int                       (T::*m_Callback)(std::vector<Arg> const& argList);
                              CommandDelegateT(T* instance, int (T::*callback)(std::vector<Arg> const& argList)) : m_Instance(instance), m_Callback(callback) { }
    virtual int invoke(std::vector<Arg> const& argList) {
                                return (m_Instance->*m_Callback)(argList);
                              }
  };

  struct Command {
                              Command(const char* name, CommandDelegate* d, const char* desc, const char* args) :
                                m_Name(name),
                                m_Delegate(d),
                                m_Description(desc),
                                m_Args(args) {
                              }
    Hue::Util::String         m_Name;
    Hue::Util::String         m_Description;
    Hue::Util::String         m_Args;
    CommandDelegate*          m_Delegate;
  };

  struct Variable {
    Hue::Util::String         m_Name;
    void*                     m_ValuePtr;
    int                       m_Value;
    int                       m_Size;
                              Variable(const char* name, int value, int size);
                              Variable(const char* name, void* value_ptr, int size);
    int                       getValue() const;
    void                      setValue(int value);
    Hue::Util::String         toString() const;
  };
private:
  struct Breakpoint {
    enum Type {
      CODE,
      WRITE,
      READ
    };
                              Breakpoint(int id, int beginaddr, int endaddr, Type type, BreakPointEvaluator* e);
                              ~Breakpoint();
    int                       m_ID;
    int                       m_BeginAddress;
    int                       m_EndAddress;
    bool                      m_Enabled;
    Type                      m_Type;
    BreakPointEvaluator*      m_Evaluator;
  };

  struct HistoryItem {
    C64MachineState::CPUState m_CPUState;
  };

  int                         m_NextBreakpointID;
  int                         m_PageSize;
  int                         m_DisasmPC;
  Hue::Util::String           m_Banner;
  Hue::Util::String           m_Prompt;
  Hue::Util::String           m_PrevCommand;
  Hue::Util::String::List     m_CommandQueue;
  C64MachineState&            m_Emu;
  GetLineDelegate*            m_GetLine;
  WriteDelegate*              m_Write;
  std::map<Hue::Util::String, Command*> 
                              m_CommandMap;
  std::map<Hue::Util::String, Variable*>
                              m_VariableMap;
  std::map<int, Breakpoint*>  m_BreakpointMap;
  bool                        m_StopInteractive;
  bool                        m_Quit;
  int                         m_BreakpointHit;
  HistoryItem*                m_HistoryBuf;
  int                         m_HistoryItems;
  int                         m_HistorySize;
  int                         m_HistoryCurrent;

  Hue::Util::String           defaultGetLine(const char* prompt);
  void                        defaultWrite(Hue::Util::String const&);
  bool                        tokenize(Hue::Util::String& cmd, std::vector<Arg>& args, const char* str);
  bool                        validateRangeInclusive(int start, int end);
  int                         createBreakpoint(std::vector<Arg> const& args, Breakpoint::Type type);
  int                         deleteBreakpoint(int id);
  int                         eraseBreakpoint(int id);
  int                         disasm(int& pc);
  Hue::Util::String           describeParams(const char* params);
  bool                        validateParams(const char* params, std::vector<Arg> const& args);
  bool                        handleBreakpointHit();
  bool                        stepEmu();
  int                         cmdHelp(std::vector<Arg> const&);
  int                         cmdExit(std::vector<Arg> const&);
  int                         cmdRegs(std::vector<Arg> const&);
  int                         cmdQuit(std::vector<Arg> const&);
  int                         cmdTest(std::vector<Arg> const&);
  int                         cmdDisassemble(std::vector<Arg> const&);
  int                         cmdMemory(std::vector<Arg> const&);
  int                         cmdAddBreakpoint(std::vector<Arg> const&);
  int                         cmdAddBreakOnWrite(std::vector<Arg> const&);
  int                         cmdAddBreakOnRead(std::vector<Arg> const&);
  int                         cmdListBreakpoints(std::vector<Arg> const&);
  int                         cmdRemoveBreakpoint(std::vector<Arg> const&);
  int                         cmdSingleStep(std::vector<Arg> const&);
  int                         cmdStepOver(std::vector<Arg> const&);
  int                         cmdJMP(std::vector<Arg> const&);
  int                         cmdHistory(std::vector<Arg> const&);
  int                         cmdAssemble(std::vector<Arg> const&);
  int                         cmdFill(std::vector<Arg> const&);
  int                         cmdHunt(std::vector<Arg> const&);
  int                         cmdSet(std::vector<Arg> const&);
public:
                              Monitor(C64MachineState& emu);
                              ~Monitor();
  void                        addCommand(const char* name, CommandDelegate* d, const char* description = NULL, const char* args = NULL);
  template<typename T>
  void                        addCommandT(const char* name, T* instance, int (T::*callback)(std::vector<Arg> const&), const char* description, const char* args) {
                                return addCommand(name, new CommandDelegateT<T>(instance, callback), description, args);
                              }
  void                        setGetLineDelegate(GetLineDelegate* d);
  template<typename T>
  void                        setGetLineDelegateT(T* instance, Hue::Util::String (T::*callback)(const char*)) {
                                setGetLineDelegate(new GetLineDelegateT<T>(instance, callback));
                              }
  void                        setWriteDelegate(WriteDelegate* d);
  template<typename T>
  void                        setWriteDelegateT(T* instance, void (T::*callback)(Hue::Util::String const&)) {
                                setWriteDelegate(new WriteDelegateT<T>(instance, callback));
                              }
  void                        writeLine(const char* str);
  void                        writeLine(Hue::Util::String const& str);
  int                         executeString(const char* str);
  int                         executeString(Hue::Util::String const& str);
  int                         executeScript(const char* filename);
  int                         interactive();
  bool                        breakpointPending();
  Hue::Util::String::List     getHistory();
  Variable*                   setVariable(const char* name, byte* valuePtr);
  Variable*                   setVariable(const char* name, word* valuePtr);
  Variable*                   setVariable(const char* name, int* valuePtr);
  Variable*                   setVariable(const char* name, int value);
  Variable*                   getVariable(const char* name);
  void                        deleteVariable(const char* name);

  friend BreakPointEvaluator;
  friend RecordExecutionHandler;
};

}

#endif