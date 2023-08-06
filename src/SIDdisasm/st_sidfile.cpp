
#include <stdlib.h>
#include <stdio.h>

#include "Types.h"
#include "st_sidfile.h"

namespace SteinTronic {

class memptr
{
public:
  memptr() : data(NULL)
  {
  }

  ~memptr()
  {
    if (this->data)
    {
      free(this->data);
      this->data = NULL;
    }
  }

  uint8_t* data;
};


int SIDFile::createSID(const char* filename, const char* name, const char* author, const char* released, int sidmodel, int songs) {
  memptr
    buf;

  int error = 0;

  Hue::Util::String
    sFilename,
    sOutfile;

  bool fallbackToDefault = true;

  sFilename = filename;
  sOutfile = sFilename;
  sOutfile.tolower();
  sOutfile.truncate_from_last(".prg");

  Hue::Util::String
    sSongName(name),
    sAuthor(author),
    sReleaseDate(released);

  bool
    isSID8580 = sidmodel != 0;

  if (sSongName.length() == 0)
  {
    if (fallbackToDefault)
    {
      sSongName = sOutfile;
    }
    else
    {
      fprintf(stderr, "ERROR: You must supply a song name.\n");
      ++error;
    }
  }
  if (sAuthor.length() == 0)
  {
    if (fallbackToDefault)
    {
      sAuthor = "Stein Pedersen/Prosonix";
    }
    else
    {
      fprintf(stderr, "ERROR: You must supply an author name.\n");
      ++error;
    }
  }
  if (sReleaseDate.length() == 0)
  {
    if (fallbackToDefault)
    {
      sReleaseDate = "1/8/2010";
    }
    else
    {
      fprintf(stderr, "ERROR: You must supply a release date.\n");
      ++error;
    }
  }
  if (error == 0)
  {
    FILE
      *pFile = fopen(sFilename.c_str(), "rb");

    bool
      isStandalone = false;

    int
      page = 0;

    if (pFile)
    {
      fseek(pFile, 0, SEEK_END);
      long size = ftell(pFile);
      fseek(pFile, 0, SEEK_SET);
      if (size > 1000)
      {
        buf.data = (uint8_t*)malloc(size);
        fread(buf.data, size, 1, pFile);
        if (buf.data[0] == 0x01 &&
            buf.data[1] == 0x08 &&
            buf.data[2] == 0x0b &&
            buf.data[3] == 0x08 &&
            buf.data[7] == '2' &&
            buf.data[8] == '0' &&
            buf.data[9] == '6' &&
            buf.data[10] == '4' &&
            buf.data[0x0895 - 0x07ff] == 0x4c &&
            buf.data[0x0898 - 0x07ff] == 0x4c &&
            buf.data[0x089b - 0x07ff] == 0x4c &&
            buf.data[0x089e - 0x07ff] == 0x4c &&
            buf.data[0x08a1 - 0x07ff] == 0xa9 &&
            1)
        {
          isStandalone = true;
        }
        else if (buf.data[2] == 0x4c &&
                 buf.data[5] == 0x4c)
        {
          page = buf.data[0] + buf.data[1] * 256;
        }
        else
        {
          fprintf(stderr, "Wrong input format error in file '%s'\n", sFilename.c_str());
          ++error;
          goto cleanup;
        }

        psidHeader
          myHeader;

        int
          init = 0,
          play = 0,
          flags = 0;

        if (isSID8580)
        {
          flags = 0x20; // sid model 6581/8580
        }
        if (isStandalone)
        {
          init = 0x0895;
          play = 0x0898;
        }
        else
        {
          init = page + 0;
          play = page + 3;
        }

        memset(&myHeader, 0, sizeof(myHeader));
        myHeader.id[0] = 'P';
        myHeader.id[1] = 'S';
        myHeader.id[2] = 'I';
        myHeader.id[3] = 'D';
        psidHeader::putwordBE(0x0002, myHeader.version);
        psidHeader::putwordBE(sizeof(myHeader), myHeader.data);
        psidHeader::putwordBE(0x0000, myHeader.load);
        psidHeader::putwordBE(init, myHeader.init);
        psidHeader::putwordBE(play, myHeader.play);
        psidHeader::putwordBE((unsigned short)songs, myHeader.songs);
        psidHeader::putwordBE(0x0001, myHeader.start);
        psidHeader::putwordBE(flags, myHeader.flags);

        if (sSongName.limit_lengthp(31))
        {
          printf("WARNING: Song name truncated to: %s\n", sSongName.c_str());
        }
        if (sAuthor.limit_lengthp(31))
        {
          printf("WARNING: Author name truncated to: %s\n", sAuthor.c_str());
        }
        if (sReleaseDate.limit_lengthp(31))
        {
          printf("WARNING: Release date truncated to: %s\n", sReleaseDate.c_str());
        }
        strcpy(myHeader.name, sSongName.c_str());
        strcpy(myHeader.author, sAuthor.c_str());
        strcpy(myHeader.released, sReleaseDate.c_str());

        Hue::Util::String
          sTemp = sOutfile;

        sTemp.tolower();
        if (!sTemp.ends_with(".sid"))
        {
          sOutfile.append(".sid");
        }

        FILE
          *pOutfile = fopen(sOutfile.c_str(), "wb");

        if (pOutfile)
        {
          fwrite(&myHeader, sizeof(myHeader), 1, pOutfile);
          fwrite(buf.data, size, 1, pOutfile);
          fclose(pOutfile);
          printf("Created file '%s'\n", sOutfile.c_str());
        }
        else
        {
          fprintf(stderr, "ERROR: Failed to create output file '%s'\n", sOutfile.c_str());
        }
      }
      else
      {
        fprintf(stderr, "Wrong input format error in file '%s'\n", sFilename.c_str());
        ++error;
      }
cleanup:
      fclose(pFile);
    }
    else
    {
      fprintf(stderr, "Failed to load file '%s'\n", sFilename.c_str());
      ++error;
    }
  }
  return error;
}

} // namespace SteinTronic
