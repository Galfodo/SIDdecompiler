
#include "testrunner.h"

namespace SASM { namespace Tests {

bool simpletest(SASM::TestInfo& test) {
  auto src = 
    "  * = $1000  \n"
    "  lda #1     \n"
    "  ldx #2     \n" 
    "  ldy #3     \n"
    "  rts        \n ";

  test.testRunner().compileAndRunString(src, "simpletest");
  auto machine = test.testRunner().machineState();
  SASM_TEST_ASSERT(test, machine->A == 1);
  SASM_TEST_ASSERT(test, machine->X == 2);
  SASM_TEST_ASSERT(test, machine->Y == 3);
  return true;
}

} }

