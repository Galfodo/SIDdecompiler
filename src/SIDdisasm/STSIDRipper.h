#ifndef STSIDRIPPER_H_INCLUDED
#define STSIDRIPPER_H_INCLUDED

#include "Types.h"
#include "STMemBuf.h"
#include <vector>

class STSIDRipper {
public:
  struct sidHeader {
    int version;
    int dataOffset;
    int loadAddress;
    int initAddress;
    int playAddress;
    int nrSongs;
    int startSong;
    int speed;
    Hue::Util::String name;
    Hue::Util::String author;
    Hue::Util::String released;
  };

  STSIDRipper(void);
  int load(const char* filename);
  ~STSIDRipper(void);

  int findString(const char* pattern, int startAddress);

  bool isLegalAddress(int address);
  bool isAddressInSIDRange(int address);

  sidHeader m_Header;
  STMemBuf m_Memory;
  int m_Size;

};

#endif
