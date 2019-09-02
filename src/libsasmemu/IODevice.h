#ifndef SASM_IODEVICE_H_INCLUDED
#define SASM_IODEVICE_H_INCLUDED

#include "Types.h"
#include "C64MachineState.h"

namespace SASM {

class IODevice : public MemoryMappedDevice {
protected:
  void configurePages(int page, int page_count, bool mirror_pages, byte* readData, byte* writeData, int pageMask, bool notify);
};

}

#endif
