#ifndef ST_SIDFILE_H_INCLUDED
#define ST_SIDFILE_H_INCLUDED

namespace SteinTronic {

typedef unsigned char uint8_t;

struct psidHeader           // all values big-endian
{
  static inline void putwordBE(unsigned short val, uint8_t* pDest) {
    pDest[0] = (val >> 8) & 0xff;
    pDest[1] = (val >> 0) & 0xff;
  }

  static inline unsigned short getwordBE(uint8_t* pSrc) {
    return ((unsigned short)pSrc[0] << 8) | pSrc[1];
  }

  char id[4];             // 'PSID' (ASCII)
  uint8_t version[2];     // 0x0001 or 0x0002
  uint8_t data[2];        // 16-bit offset to binary data in file
  uint8_t load[2];        // 16-bit C64 address to load file to
  uint8_t init[2];        // 16-bit C64 address of init subroutine
  uint8_t play[2];        // 16-bit C64 address of play subroutine
  uint8_t songs[2];       // number of songs
  uint8_t start[2];       // start song out of [1..256]
  uint8_t speed[4];       // 32-bit speed info
                          // bit: 0=50 Hz, 1=CIA 1 Timer A (default: 60 Hz)
  char name[32];          // ASCII strings, 31 characters long and
  char author[32];        // terminated by a trailing zero
  char released[32];      //
  uint8_t flags[2];       // only version 0x0002
  uint8_t relocStartPage; // only version 0x0002B
  uint8_t relocPages;     // only version 0x0002B
  uint8_t reserved[2];    // only version 0x0002

};

class SIDFile {
public:
  static int createSID(const char* filename, const char* name, const char* author, const char* released, int sidmodel, int songs);
};

} // namespace SteinTronic

#endif
