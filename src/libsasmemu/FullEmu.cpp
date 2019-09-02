
#include "FullEmu.h"
#include "CIA.h"
#include "VIC-II.h"
#include "SID.h"
#include "ROM.h"
#include "PLA.h"

namespace SASM {

FullEmu::FullEmu() {
  attach(new PLA());
  attach(new CIA(CIA::CIA_1));
  attach(new CIA(CIA::CIA_2));
  attach(new VICII());
  attach(new SID());
  attach(new BasicROM());
  attach(new KernalROM());
  attach(new ChargenROM());
  softReset();
}

}