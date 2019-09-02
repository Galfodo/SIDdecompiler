
#include "IODevice.h"

namespace SASM {

void IODevice::configurePages(int page, int page_count, bool mirror_pages, byte* readData, byte* writeData, int pageMask, bool notify) {
  machine().m_ReadConfig[0x05].configurePages (page, page_count, mirror_pages, readData,  notify ? this : nullptr, pageMask);
  machine().m_WriteConfig[0x05].configurePages(page, page_count, mirror_pages, writeData, notify ? this : nullptr, pageMask);
  machine().m_ReadConfig[0x06].configurePages (page, page_count, mirror_pages, readData,  notify ? this : nullptr, pageMask);
  machine().m_WriteConfig[0x06].configurePages(page, page_count, mirror_pages, writeData, notify ? this : nullptr, pageMask);
  machine().m_ReadConfig[0x07].configurePages (page, page_count, mirror_pages, readData,  notify ? this : nullptr, pageMask);
  machine().m_WriteConfig[0x07].configurePages(page, page_count, mirror_pages, writeData, notify ? this : nullptr, pageMask);
}

}
