
#include "testrunner.h"
#include "Assembler.h"
#include "CIA.h"
#include <assert.h>
#include <algorithm>

#ifdef _MSC_VER
#include <direct.h>
#else
#include <unistd.h>
#define _getcwd getcwd
#endif

#define IMPORT_TEST(name) extern bool name(SASM::TestInfo&)

namespace SASM {

TestInfo::TestInfo(TestRunner& testRunner, const char* name) : m_TestRunner(testRunner), m_Name(name) {
}


void TestInfo::assertionError(const char* expr, const char* file, int line) {
  addError(Hue::Util::String::static_printf("Error in (%s:%d) : \n  ASSERTION FAILED : '%s'", file, line, expr));
}

TestRunner::TestRunner(const char* test_dir) : m_TestDir(test_dir), m_CurrentTest(nullptr) {
}

bool TestRunner::runTest(TestRunner::test_pf pfTest, const char* name) {
  assert(pfTest);
  m_MachineState.reset(new SASM::FullEmu());
  m_MachineState->attach(new CIA(CIA::CIA_1));
  auto testInfo = new TestInfo(*this, name);
  m_Tests.push_back(std::unique_ptr<TestInfo>(testInfo));
  m_CurrentTest = testInfo;
  pfTest(*testInfo);
  m_CurrentTest = nullptr;
  if (testInfo->success()) {
    printf("'%s': PASSED\n", testInfo->name());
  } else {
    std::for_each(testInfo->errors().begin(), testInfo->errors().end(), [ testInfo ](auto &error) {
      fprintf(stderr, "'%s': %s\n", testInfo->name(), error.c_str());
    });
    m_FailedTests += 1;
  }
  return testInfo->success();
}

bool TestRunner::compileString(std::vector<byte>& out_data, const char* src, const char* srcname) {
  m_Assembler.reset(new SASM::Assembler);
  m_Assembler->m_Quiet = true;
  out_data = m_Assembler->assemble(src, srcname);
  if (m_Assembler->errorcount()) {
    m_CurrentTest->addError(Hue::Util::String::static_printf("Assembler error(s): %d", m_Assembler->errorcount()));
  } 
  return m_Assembler->errorcount() == 0;
}

bool TestRunner::compileAndRunString(const char* src, const char* srcname) {
  std::vector<byte> prg_data;
  if (compileString(prg_data, src, srcname)) {
    m_MachineState->load((int)m_Assembler->m_ORG, &prg_data[0], (int)prg_data.size(), true);
    do {
    } while (m_MachineState->runCPU());
    return true;
  }
  return false;
}

SASM::FullEmu* TestRunner::machineState() {
  return m_MachineState.get();
}

}

#define RUN_TEST(name) runner.runTest(SASM::Tests::name, #name)

namespace SASM { namespace Tests {
  IMPORT_TEST(simpletest);
  IMPORT_TEST(brktest);
  IMPORT_TEST(timerirqtest);
} }


int main(int argc, char** argv) {
  Hue::Util::String path;
  path.reallocate(2048, false);
  (void)_getcwd(path.buffer(), 2048);
  path.replace("\\", "/");
  path.truncate_after_last("/");
  path.append("src/sasmtestrunner/");

  SASM::TestRunner
    runner(path);

  printf("SASM Test Runner:\n");
  printf("-----------------\n");
  RUN_TEST(simpletest);
  RUN_TEST(brktest);
  RUN_TEST(timerirqtest);
  printf("-----------------\n");
  printf("Test run complete with %d failed test%s.\n", runner.failedTests(), runner.failedTests() == 1 ? "" : "s");
  return runner.failedTests() ? -1 : 0;
}
