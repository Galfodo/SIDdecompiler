
#include "testrunner.h"


namespace SASM { namespace Tests {

bool rmwtest(SASM::TestInfo& test) {
  // Test that read-modify-write instructions read from ROM and write to RAM
  auto src = 
    "  * = $1000                \n"
    "  inc $e000                \n"
    "  ldx $e000                \n"
    "  lda #$35                 \n"
    "  sta $01                  \n"
    "  lda $e000                \n"
    "  rts                      \n";

  test.testRunner().compileAndRunString(src, "rmwtest");
  auto machine = test.testRunner().machineState();
  SASM_TEST_ASSERT(test, machine->X == 0x85);
  SASM_TEST_ASSERT(test, machine->A == machine->X + 1);
  return true;
}

bool iotest(SASM::TestInfo& test) {
  // Test that writing to the VIC is only reflected there and not the underlying RAM
  auto src = 
    "  * = $1000                \n"
    "  lda #$55                 \n"
    "  sta $d000                \n"
    "  ldx $d000                \n"
    "  lda #$34                 \n"
    "  sta $01                  \n"
    "  lda $d000                \n"
    "  rts                      \n";

  test.testRunner().compileAndRunString(src, "iotest");
  auto machine = test.testRunner().machineState();
  SASM_TEST_ASSERT(test, machine->X == 0x55);
  SASM_TEST_ASSERT(test, machine->A == 0x00);
  return true;
}

} }
