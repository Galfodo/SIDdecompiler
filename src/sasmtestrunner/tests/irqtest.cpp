
#include "testrunner.h"

namespace SASM { namespace Tests {

bool irqtest(SASM::TestInfo& test) {
  auto src = 
    "  * = $1000     \n"
    "  lda #<irq     \n"
    "  ldy #>irq     \n"
    "  sta $fffe     \n"
    "  sty $ffff     \n"
    "  ldx #0        \n"
    "  lda #0        \n"
    "  brk           \n" // When a BRK instruction is executed, PC+2 is pushed to the stack
    "  inx           \n" // <- so when RTI occurs, this instruction will be skipped
    "  rts           \n" // RTI returns here
    "irq:            \n"
    "  lda #1        \n"
    "  rti           \n";

  test.testRunner().compileAndRunString(src, "brktest");
  auto machine = test.testRunner().machineState();
  SASM_TEST_ASSERT(test, machine->A == 1);
  SASM_TEST_ASSERT(test, machine->X == 0);
  return true;
}

} }

