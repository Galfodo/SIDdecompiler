
#include "testrunner.h"

namespace SASM { namespace Tests {

bool brktest(SASM::TestInfo& test) {
  auto src = 
    "  * = $1000     \n"
    "  lda #<irq     \n"
    "  ldy #>irq     \n"
    "  sta $fffe     \n"
    "  sty $ffff     \n"
    "  lda #$35      \n"
    "  sta $01       \n"
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

bool timerirqtest(SASM::TestInfo& test) {
  auto src = 
    "  * = $1000     \n"
    "  lda #$35      \n"
    "  sta $01       \n"
    "  lda #<irq     \n"
    "  ldy #>irq     \n"
    "  sta $fffe     \n"
    "  sty $ffff     \n"
    "  ldx #0        \n"
    "  lda #100      \n"
    "  sta $dc04     \n"
    "  stx $dc05     \n"
    "  lda #$81      \n"
    "  sta $dc0d     \n"
    "  lda #$11      \n"
    "  sta $dc0e     \n"
    "  ldy #$00      \n"
    "loop:           \n"
    "  inx           \n"
    "  bne loop      \n"
    "  rts           \n"
    "irq:            \n"
    "  tya           \n"
    "  beq noack     \n" // We want the interrupt to occur twice, so we only acknowledge it on the second occurrence
    "  lda #$7f      \n"
    "  sta $dc0d     \n"
    "  lda $dc0d     \n"
    "noack:          \n"
    "  iny           \n"
    "  rti           \n";

  test.testRunner().compileAndRunString(src, "timerirqtest");
  auto machine = test.testRunner().machineState();
  SASM_TEST_ASSERT(test, machine->Y == 2);
  SASM_TEST_ASSERT(test, machine->A == 0x81);
  return true;
}


} }

