#ifndef SASM_TESTRUNNER_H_INCLUDED
#define SASM_TESTRUNNER_H_INCLUDED

#include "FullEmu.h"
#include <vector>
#include <memory>

namespace SASM { 

#define SASM_TEST_ASSERT(test, expr) do {              \
    if (!(expr)) {                                     \
      test.assertionError(#expr, __FILE__, __LINE__);  \
    }                                                  \
  } while(0)

class TestInfo
{
  Hue::Util::String              m_Name;
  std::vector<Hue::Util::String> m_Errors;
  class TestRunner&              m_TestRunner;
public:
                      TestInfo(TestRunner& testrunner, const char* name);
  void                assertionError(const char* expr, const char* file, int line);
  inline void         addError(Hue::Util::String const& msg) { m_Errors.push_back(msg); }
  inline bool         success() const { return m_Errors.empty(); }
  inline std::vector<Hue::Util::String> const&
                      errors() const { return m_Errors; }
  inline const char*  name() const { return m_Name.c_str(); }
  inline TestRunner&  testRunner() { return m_TestRunner; }
};

class TestRunner {

  std::unique_ptr<SASM::FullEmu> 
                    m_MachineState;

  std::unique_ptr<SASM::Assembler>
                    m_Assembler;

  std::vector<std::unique_ptr<SASM::TestInfo> >
                    m_Tests;

  Hue::Util::String m_TestDir;

  SASM::TestInfo*   m_CurrentTest;

  int               m_FailedTests;

public:
  typedef bool (*test_pf)(TestInfo& );
                    TestRunner(const char* test_dir);
  bool              runTest(test_pf pfTest, const char* name = "<unnamed test>");
  bool              compileString(std::vector<byte>& out_data, const char* src, const char* srcname = "<string>");
  bool              compileAndRunString(const char* src, const char* srcname = "<string>");
  FullEmu*          machineState();
  inline int        failedTests() { return m_FailedTests; }
};

}

#endif
