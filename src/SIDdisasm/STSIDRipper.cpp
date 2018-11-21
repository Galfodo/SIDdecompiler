#include "STSIDRipper.h"
#include "st_sidfile.h"
#include "STHubbardRipper.h"

#include <vector>

STSIDRipper::STSIDRipper(void)
{
}

STSIDRipper::~STSIDRipper(void)
{
}

int STSIDRipper::load(const char* filename) {
  memset(&m_Header, 0, sizeof(m_Header));
  STMemBuf mem;
  if (mem.load(filename) == 0) {
    if (mem.size() > sizeof(SteinTronic::psidHeader)) {
      SteinTronic::psidHeader* psid = (SteinTronic::psidHeader*)mem.data();
      sidHeader header;
      header.author       = psid->author;
      header.name         = psid->name;
      header.released     = psid->released;
      header.version      = SteinTronic::psidHeader::getwordBE(psid->version);
      header.dataOffset   = SteinTronic::psidHeader::getwordBE(psid->data);
      header.loadAddress  = SteinTronic::psidHeader::getwordBE(psid->load);
      header.initAddress  = SteinTronic::psidHeader::getwordBE(psid->init);
      header.playAddress  = SteinTronic::psidHeader::getwordBE(psid->play);
      header.nrSongs      = SteinTronic::psidHeader::getwordBE(psid->songs);
      header.startSong    = SteinTronic::psidHeader::getwordBE(psid->start);
      header.speed        = ((int)psid->speed[0] << 24) | ((int)psid->speed[1] << 16);
      if (header.loadAddress == 0) {
        header.loadAddress = *(unsigned short*)(mem.data() + header.dataOffset);
        header.dataOffset += 2;
      }
      m_Header = header;
      m_Memory.resize(65536);
      memset(m_Memory.data(), 0xff, 65536);
      m_Size = mem.size() - header.dataOffset;
      memcpy(m_Memory.data() + header.loadAddress, mem.data() + header.dataOffset, m_Size);
      return 0;
    }
  }
  return -1;
}

int STSIDRipper::findString(const char* pattern, int startAddress) {
  if (startAddress == 0) {
    startAddress = m_Header.loadAddress;
  } else {
    assert(startAddress >= m_Header.loadAddress);
  }
  int delta = startAddress - m_Header.loadAddress;
  int currentSize = m_Size - delta;
  std::vector<int> matchstring;
  while (*pattern) {
    if (*pattern == '?' || *pattern == '*') {
      matchstring.push_back(-1);
      pattern += 2;
    } else {
      int value = 0;
      for (int i = 0; i < 2; ++i) {
        value <<= 4;
        if (*pattern >= 'a')
          value |= (*pattern - 'a') + 0x0a;
        else 
          value |= *pattern & 0x0f;
        ++pattern;
      }
      matchstring.push_back(value);
    }
  }
  for (int i = 0; i < currentSize; ++i) {
    bool match = true;
    for (int x = 0; x < (int)matchstring.size(); ++x) {
      if (matchstring[x] >= 0) {
        if (m_Memory[startAddress + i + x] != matchstring[x]) {
          match = false;
          break;
        }
      }
    }
    if (match)
      return i + startAddress;
  }
  return -1;
}

bool STSIDRipper::isLegalAddress(int address) {
  if (address < 0)
    return false;
  if (address >= 65536)
    return false;
  return true;
}

bool STSIDRipper::isAddressInSIDRange(int address) {
  if (address < m_Header.loadAddress)
    return false;
  if (address >= m_Header.loadAddress + m_Size)
    return false;
  return true;
}
