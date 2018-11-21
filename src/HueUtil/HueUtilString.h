#ifndef HUE_UTIL_STRING_H_INCLUDED
#define HUE_UTIL_STRING_H_INCLUDED

// File version 123. Changed 31.01.2017

#include <string>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "RegExp.h" // FIXMEREMOVEME

#ifndef HUE_USE_FAST_STRING_SEARCH
#define HUE_USE_FAST_STRING_SEARCH 1
#endif

#ifdef _MSC_VER
  #ifndef va_copy
    #define va_copy(dest, src) (dest) = (src)
  #endif
#endif

#ifdef _MSC_VER
  #define HUE_STRCMP strcmp
  #define HUE_STRICMP _stricmp
#else
  #define HUE_STRCMP strcmp
  #define HUE_STRICMP strcasecmp
#endif

namespace Hue
{
  namespace Util
  {
    typedef long long Int64;

    template<bool ISCASEINSENSITIVE>
    Int64  stringHashT(const char* pzString, size_t length);
    template<>
    Int64  stringHashT<false>(const char* pzString, size_t length);
    template<>
    Int64  stringHashT<true>(const char* pzString, size_t length);

    // This string class is heavily based on thomas DocString_c class, optimized with regard to memory allocation count,
    // API more std::string-like
    class String
    {
    public:
      // List of Strings
      class List
      {
      public:
        List() : _Strings(0), _Count(0)
        {
        }

        List(const List& rhs) : _Strings(0), _Count(0)
        {
          *this = rhs;
        }

        List(int count) : _Strings(0), _Count(0)
        {
          reallocate(count);
        }

        List(const char** string_array, int count) : _Strings(0), _Count(0)
        {
          append_strings(string_array, count);
        }

        List(const char** null_terminated_string_array) : _Strings(0), _Count(0)
        {
          int count = 0;
          for (; null_terminated_string_array[count]; ++count) ;
          append_strings(null_terminated_string_array, count);
        }

        ~List()
        {
          clear();
        }

        List& append_strings(const char** string_array, int count)
        {
          int index = this->count();
          reallocate(count + index);
          for (int i = 0; i < count; ++i)
          {
            ((*this)[i + index]) = string_array[i];
          }
          return *this;
        }

        List& reallocate(int count)
        {
          String* newstrings = new String[count];
          for (int i = 0; i < this->_Count && i < count; ++i)
          {
            newstrings[i] = _Strings[i];
          }
          delete[] _Strings;
          _Strings = newstrings;
          _Count = count;
          return *this;
        }

        List& clear()
        {
          delete[] _Strings;
          _Strings = NULL;
          _Count = 0;
          return *this;
        }

        int count() const
        {
          return _Count;
        }

        int size() const
        {
          return count();
        }

        List& append(const String& str)
        {
          String tmp = str; // Reallocate this, because it may be a member of the List, in which case it will be invalidated by the call to reallocate()
          reallocate(_Count + 1);
          (*this)[_Count - 1] = tmp;
          return *this;
        }

        List& push_back(const String& str)
        {
          append(str);
          return *this;
        }

        List& insert(const String& str, int index)
        {
          if (index >= 0 && index <= _Count)
          {
            int tmpindex = _Count;
            reallocate(_Count + 1);
            while (tmpindex > index)
            {
              (*this)[tmpindex] = (*this)[tmpindex - 1];
              --tmpindex;
            }
            (*this)[index] = str;
          }
          return *this;
        }

        List& prepend(const String& str)
        {
          return insert(str, 0);
        }

        int empty_count() const
        {
          int count = 0;
          for (int i = 0; i < this->count(); ++i)
          {
            if ((*this)[i].empty())
            {
              ++count;
            }
          }
          return count;
        }

        List& remove(int index)
        {
          if (index >= 0 && index < _Count)
          {
            while (index < (_Count - 1))
            {
              (*this)[index] = (*this)[index + 1];
              ++index;
            }
            --_Count;
          }
          return *this;
        }

        List& remove(const String& str)
        {
          return remove(index_of(str));
        }

        List& remove_empty()
        {
          int count = 0;
          if (this->empty_count() > 0)
          {
            List tmp;
            for (int i = 0; i < this->count(); ++i)
            {
              if (!(*this)[i].empty())
              {
                tmp.append((*this)[i]);
              }
            }
            count = this->count() - tmp.count();
            *this = tmp;
          }
          return *this;
        }

        int index_of(const String& str)
        {
          for (int i = 0; i < this->count(); ++i)
          {
            if ((*this)[i] == str)
            {
              return i;
            }
          }
          return -1;
        }

        bool contains(const String& str)
        {
          return index_of(str) >= 0;
        }

        List& append_if_not_contained(const String& str)
        {
          if (!this->contains(str))
          {
            this->append(str);
          }
          return *this;
        }

        List& insert_if_not_contained(const String& str, int index)
        {
          if (!this->contains(str))
          {
            this->insert(str, index);
          }
          return *this;
        }

        String join(const char* separator = "") const
        {
          String work;
          for (int i = 0; i < size(); ++i) 
          {
            if (i > 0 && separator != NULL)
            {
              work.append(separator);
            }
            work.append(this->at(i));
          }
          return work;
        }

        List& prepend_all(const char* str) 
        {
          for (int i = 0; i < size(); ++i) 
          {
            this->at(i).prepend(str);
          }
          return *this;
        }

        List& prepend_all(const String& str) 
        {
          return prepend_all(str.c_str());
        }

        List& append_all(const char* str) 
        {
          for (int i = 0; i < size(); ++i) 
          {
            this->at(i).append(str);
          }
          return *this;
        }

        List& append_all(const String& str) 
        {
          return append_all(str.c_str());
        }

        String& operator[] (int index) const
        {
          return _Strings[index];
        }

        String& at(int index) const
        {
          return _Strings[index];
        }

        String& first() const
        {
          return at(0);
        }

        String& last() const
        {
          return at(size() - 1);
        }

        List& operator= (const List& rhs)
        {
          reallocate(rhs.count());
          for (int i = 0; i < rhs.count(); ++i)
          {
            (*this)[i] = rhs[i];
          }
          return *this;
        }

        bool operator== (const List& rhs) const
        {
          if (this->count() == rhs.count())
          {
            for (int i = 0; i < this->count(); ++i)
            {
              if ((*this)[i] != rhs[i])
              {
                return false;
              }
            }
            return true;
          }
          return false;
        }

        bool operator!= (const List& rhs) const
        {
          return !(*this == rhs);
        }

        String* begin() const
        {
          return _Strings;
        }

        String* end() const
        {
          return _Strings + _Count;
        }

        bool loadLines(const char* filename, bool include_newlines = false, bool trim_leading_and_trailing_whitespace = false)
        {
          Hue::Util::String contents;
          if (contents.load(filename))
          {
            Hue::Util::String::List lines = contents.split("\n", include_newlines, trim_leading_and_trailing_whitespace);
            *this = lines;
            return true;
          }
          return false;
        }

      private:
        String* _Strings;
        int     _Count;
      };

#if HUE_USE_FAST_STRING_SEARCH
      enum { FastSearchLengthThreshold = 8 };

      static const char*   
                    fast_findfirst(const char* pzSearch, size_t nSearchLength, const char * pzPattern, size_t nPatternLength);
      static const char*   
                    fast_findfirstcaseinsensitive(const char* pzSearch, size_t nSearchLength, const char * pzPattern, size_t nPatternLength);
      static const char*   
                    fast_findlast(const char* pzSearch, size_t nSearchLength, const char * pzPattern, size_t nPatternLength);
      static int    fast_replace(char* pzNewBuffer, const char* pzSearch, size_t nSearchLength, const char * pzNewPattern, size_t nNewPatternLength, const char * pzOldPattern, size_t nOldPatternLength);
#endif

      // Methods:
                    String() :
                      _pzASCII(0),
                      _pzALLOCATED(0),
                      _nSize(0),
                      _nLength(0),
                      _nHash(0)
                    {
                    }

                    String(const char * pzASCII) :
                      _pzASCII(0),
                      _pzALLOCATED(0),
                      _nSize(0),
                      _nLength(0),
                      _nHash(0)
                    {
                      assign(pzASCII);
                    }
                    String(const char * pzASCII, size_t nLength) :
                      _pzASCII(0),
                      _pzALLOCATED(0),
                      _nSize(0),
                      _nLength(0),
                      _nHash(0)
                    {
                      assign(pzASCII, pzASCII + nLength);
                    }

                    String(const char * pzBegin, const char* pzEnd) :
                      _pzASCII(0),
                      _pzALLOCATED(0),
                      _nSize(0),
                      _nLength(0),
                      _nHash(0)
                    {
                      assign(pzBegin, pzEnd);
                    }

                    String(const String& String) :
                      _pzASCII(0),
                      _pzALLOCATED(0),
                      _nSize(0),
                      _nLength(0),
                      _nHash(0)
                    {
                      assign(&String);
                    }

                    String(const String * pcDocString) :
                      _pzASCII(0),
                      _pzALLOCATED(0),
                      _nSize(0),
                      _nLength(0),
                      _nHash(0)
                    {
                      assign(pcDocString);
                    }

                    String(const std::string& str) :
                      _pzASCII(0),
                      _pzALLOCATED(0),
                      _nSize(0),
                      _nLength(0),
                      _nHash(0)
                    {
                      assign(str.c_str());
                    }

                   ~String()
                    {
                      delete [] _pzALLOCATED;
                      _pzALLOCATED = NULL;
                      _pzASCII = NULL;
                      _nSize = 0;
                    }

      void          reallocate(size_t nSize, bool retainOldData)
                    {
                      ++nSize;
                      if (_pzASCII == NULL)
                      {
                        _pzASCII = _azTemp;
                        _azTemp[0] = '\0';
                        _nSize = sizeof(_azTemp);
                      }
                      if (nSize > _nSize)
                      {
                        size_t nRealSize = nSize * 2;
                        _nSize = nRealSize;
                        char* pzOld = _pzASCII;
                        char* pzOldALLOCATED = _pzALLOCATED;
                        _pzASCII = _pzALLOCATED = new char[nRealSize];
                        if (retainOldData && pzOld)
                        {
                          strcpy(_pzASCII, pzOld);
                        }
                        else
                        {
                          _pzASCII[0] = '\0';
                        }
                        delete [] pzOldALLOCATED;
                      }
                    }

                    // Create a wrapper without allocating space. Use at your own peril!
      void          create_wrapper(const char* pzString)
                    {
                      this->~String();
                      this->_nHash = 0;
                      this->_nLength = -1;
                      this->_pzASCII = (char*)pzString;
                    }

      void          create_wrapper(std::string const& str)
                    {
                      this->~String();
                      this->_nHash = 0;
                      this->_nLength = -1;
                      this->_pzASCII = (char*)str.c_str();
                    }

      char*         buffer() const
                    {
                      return _pzASCII;
                    }

      String&       clear()
                    {
                      if (_pzASCII)
                      {
                        *_pzASCII = '\0';
                      }
                      _nHash = 0;
                      _nLength = 0;
                      return *this;
                    }

      const char *  c_str() const
                    {
                      if (!_pzASCII)
                      {
                        return "";
                      }
                      return _pzASCII;
                    }

      int           length() const
                    {
                      if (_nLength < 0)
                      {
                        if (_pzASCII)
                        {
                          _nLength = (int)strlen(_pzASCII);
                        }
                        else
                        {
                          _nLength = 0;
                        }
                      }
                      return _nLength;
                    }

      String&       assign(const String * pcDocString)
                    {
                      if (pcDocString)
                      {
                        assign(pcDocString->_pzASCII);
                      }
                      else
                      {
                        clear();
                      }
                      return *this;
                    }

      String&       assign(const String& String)
                    {
                      return assign(String._pzASCII);
                    }

      String&       assign(const std::string& str)
                    {
                      return assign(str.c_str());
                    }

      String&       assign(const char * pzASCII)
                    {
                      if (!pzASCII)
                      {
                        clear();
                        return *this;
                      }
                      int len = (int)strlen(pzASCII);
                      reallocate(len, false);
                      strcpy(_pzASCII, pzASCII);
                      _nLength = len;
                      _nHash = 0;
                      return *this;
                    }

      String&       assign(const char * pzSubStringStart, const char * pzSubStringEnd)
                    {
                      if (pzSubStringEnd > pzSubStringStart)
                      {
                        size_t nSize = (size_t)(pzSubStringEnd - pzSubStringStart);
                        reallocate(nSize, false);
                        strncpy(_pzASCII, pzSubStringStart, nSize);
                        _pzASCII[nSize] = '\0';
                        _nLength = (int)nSize;
                        _nHash = 0;
                      }
                      else
                      {
                        clear();
                      }
                      return *this;
                    }

      bool          load(const char * pzFilename)
                    {
                      clear();
                      FILE* pcFile = fopen(pzFilename, "rb");
                      if (!pcFile)
                      {
                        return false;                    
                      }
                      fseek(pcFile, 0, SEEK_END);
                      size_t nSize = ftell(pcFile);
                      if (!nSize)
                      {
                        fclose(pcFile);
                        return false;
                      }
                      reallocate(nSize, false);
                      fseek(pcFile, 0, SEEK_SET);
                      size_t nRead = fread(_pzASCII, 1, nSize, pcFile);
                      fclose(pcFile);
                      _pzASCII[nSize] = 0;
                      _nLength = (int)nSize;
                      _nHash = 0;
                      if (nRead != nSize)
                      {
                        clear();
                        return false;
                      }
                      return true;
                    }

      bool          save(const char * pzFilename) const
                    {
                      FILE * pcFile = fopen(pzFilename, "wb");
                      if (!pcFile)
                      {
                        return false;
                      }
                      if (length())
                      {
                        fwrite(_pzASCII, 1, length(), pcFile);
                      }
                      fclose(pcFile);
                      return true;
                    } 

      static int    static_replace(char* pzNewBuffer, const char* pzSource, const char * pzNew, const char * pzOld)
                    {
                      if (pzSource == NULL)
                        return 0;
                      size_t
                        searchLen = strlen(pzSource);
                      size_t
                        oldPatternLen = strlen(pzOld);
                      size_t
                        newPatternLen = strlen(pzNew);
                      if (searchLen == 0)
                        return 0;
                      if (oldPatternLen == 0)
                        return 0;
#if HUE_USE_FAST_STRING_SEARCH
                      if (searchLen >= FastSearchLengthThreshold)
                      {
                        return fast_replace(pzNewBuffer, pzSource, searchLen, pzNew, newPatternLen, pzOld, oldPatternLen);
                      }
                      else
#endif
                      {
                        int
                          nRemaining = (int)searchLen,
                          nOldSize = (int)oldPatternLen,
                          nNewSize = (int)newPatternLen;

                        int
                          nChanged = 0;
                      
                        do
                        {
                          // replace at this position?
                          bool
                            isReplace = false;

                          if (nRemaining >= nOldSize)
                          {
                            isReplace = true;

                            for (int iScan = 0; iScan < nOldSize && isReplace; iScan++)
                            {
                              char
                                zWanted  = pzOld[iScan],
                                zPresent = pzSource[iScan];

                              if (zWanted != zPresent)
                              {
                                isReplace = false;
                              }
                            }
                          }

                          if (isReplace)
                          {
                            if (pzNewBuffer)
                            {
                              for (int iCopy = 0; iCopy < nNewSize; iCopy++)
                              {
                                pzNewBuffer[iCopy] = pzNew[iCopy];
                              }

                              pzNewBuffer[nNewSize] = 0;
                              pzNewBuffer += nNewSize;
                            }

                            pzSource += nOldSize;
                            nRemaining -= nOldSize;
                          
                            nChanged++;
                          }
                          else
                          {
                            // Copy single character:
                            if (pzNewBuffer)
                            {
                              pzNewBuffer[0] = pzSource[0];
                              pzNewBuffer[1] = 0;
                          
                              pzNewBuffer++;
                            }

                            pzSource++;

                            nRemaining--;
                          }
                        } while (nRemaining > 0);

                        return nChanged;
                      }
                    }

      const char*   findfirst(const char * pzASCII) const
                    {
                      size_t
                        searchLen = length();
                      size_t
                        patternLen = strlen(pzASCII);
                      if (searchLen == 0)
                        return NULL;
                      if (patternLen == 0)
                        return NULL;
#if HUE_USE_FAST_STRING_SEARCH
                      if (searchLen >= FastSearchLengthThreshold) 
                      {
                        return fast_findfirst(_pzASCII, searchLen, pzASCII, patternLen);
                      }
                      else
#endif
                      {
                        return strstr(_pzASCII, pzASCII);
                      }
                    }

      static const char*   
                    findfirstcaseinsensitive(const char* pzSearch, const char * pzASCII) 
                    {
                      if (pzASCII == 0) 
                        return NULL;
                      if (pzASCII == 0)
                        return NULL;
                      size_t
                        searchLen = strlen(pzSearch);
                      size_t
                        patternLen = strlen(pzASCII);
                      if (searchLen == 0)
                        return NULL;
                      if (patternLen == 0)
                        return NULL;
#if HUE_USE_FAST_STRING_SEARCH
                      if (searchLen >= FastSearchLengthThreshold)
                      {
                        return fast_findfirstcaseinsensitive(pzSearch, searchLen, pzASCII, patternLen);
                      } 
                      else 
#endif
                      {
                        String
                          tmp1(pzSearch),
                          searchfor(pzASCII);

                        tmp1.toupper();
                        searchfor.toupper();
                        const char* pzFound = tmp1.findfirst(searchfor.c_str());
                        if (pzFound)
                          return pzSearch + (pzFound - tmp1.c_str());
                        return 0;
                      }
                    }

      const char*   findfirstcaseinsensitive(const char * pzASCII) const
                    {
                      size_t
                        searchLen = length();
                      size_t
                        patternLen = strlen(pzASCII);
                      if (searchLen == 0)
                        return NULL;
                      if (patternLen == 0)
                        return NULL;
#if HUE_USE_FAST_STRING_SEARCH
                      if (searchLen >= FastSearchLengthThreshold)
                      {
                        return fast_findfirstcaseinsensitive(_pzASCII, searchLen, pzASCII, patternLen);
                      }
                      else
#endif
                      return findfirstcaseinsensitive(c_str(), pzASCII);
                    }

      const char*   findlast(const char* pzASCII) const
                    {
                      size_t
                        searchLen = length();
                      size_t
                        patternLen = strlen(pzASCII);
                      if (searchLen == 0)
                        return NULL;
                      if (patternLen == 0)
                        return NULL;
#if HUE_USE_FAST_STRING_SEARCH
                      if (searchLen >= FastSearchLengthThreshold)
                      {
                        return fast_findlast(_pzASCII, searchLen, pzASCII, patternLen);
                      }
                      else
#endif
                      {
                        const char* pzWork = strstr(_pzASCII, pzASCII);
                        const char* pzLast = pzWork;
                        while (pzWork)
                        {
                          pzWork = strstr(pzWork + 1, pzASCII);
                          if (pzWork)
                          {
                            pzLast = pzWork;
                          }
                        }
                        return pzLast;
                      }
                    }

      static Int64  hash(const char* pzString, size_t length)
                    {
                      return Hue::Util::stringHashT<false>(pzString, length);
                    }

      Int64         hash() const
                    {
                      if (_nHash == 0)
                      {
                        _nHash = hash(this->_pzASCII, this->length());
                      }
                      return _nHash;
                    }

      bool          contains(const char* pzASCII) const
                    {
                      return findfirst(pzASCII) != NULL;
                    }

      bool          equal(const String& rhs) const
                    {
                      if (hash() != rhs.hash())
                      {
                        return false;
                      }
                      return HUE_STRCMP(this->c_str(), rhs.c_str()) == 0;
                    }

      bool          equal(const String * str) const
                    {
                      return equal(*str);
                    }

      bool          equal(const char * pzChar) const
                    {
                      if (length())
                      {
                        if (!pzChar)
                        {
                          return false;
                        }
                        return !HUE_STRCMP(c_str(), pzChar);
                      }
                      else
                      {
                        if (!pzChar || !pzChar[0])
                        {
                          return true;
                        }
                        return false;
                      }
                    }

      String        substring(int startIndex)
                    {
                      Hue::Util::String
                        sSubString(this->c_str() + startIndex);

                      return sSubString;
                    }

      String        substring(int startIndex, size_t nLength)
                    {
                      Hue::Util::String
                        sSubString(this->c_str() + startIndex, nLength);

                      return sSubString;
                    }

      /* The following functions perform conditional modifications on the string. They come in two flavors:
         <operation>(args) - returns *this
         <operation>p(args) - returns true/non-zero if the string was modified and false/0 otherwise
      */
      /* conditional modification functions --> */
      String&       replace(const char* pzOld, const std::string& sNew)
                    {
                      return replace(pzOld, sNew.c_str());
                    }
      int           replacep(const char* pzOld, const std::string& sNew)
                    {
                      return replacep(pzOld, sNew.c_str());
                    }

      String&       replace(const char* pzOld, const String& sNew)
                    {
                      return replace(pzOld, sNew.c_str());
                    }
      int           replacep(const char* pzOld, const String& sNew)
                    {
                      return replacep(pzOld, sNew.c_str());
                    }

      String&       replace(const char * pzOld, const char * pzNew)
                    {
                      replacep(pzOld, pzNew);
                      return *this;
                    }
      int           replacep(const char * pzOld, const char * pzNew)
                    {
                      int
                        nChanged = static_replace(NULL, _pzASCII, pzNew, pzOld);

                      if (!nChanged)
                      {
                        return 0;
                      }

                      int
                        nNewSize = (int)strlen(pzNew),
                        nOldSize = (int)strlen(pzOld);

                      int nDeltaSize = nChanged * (nNewSize - nOldSize);
                      String cTemp;
                      size_t reallocLen = length() + nDeltaSize;
                      cTemp.reallocate(reallocLen, false);
                      cTemp._nLength = length() + nDeltaSize;
                      static_replace(cTemp._pzASCII, _pzASCII, pzNew, pzOld);
                      assign(cTemp);
                      return nChanged;
                    }

                    // Find substring matching "pattern" and substitute it for "replacement"
                    // /0 to /9 will be substituted with match substrings 0 to 9
      String&       replaceregexp(const char* pzPattern, const char* pzReplacement, bool isIgnoreCase, bool replaceFirstOccurrenceOnly = false)
                    {
                      replaceregexpp(pzPattern, pzReplacement, isIgnoreCase, replaceFirstOccurrenceOnly);
                      return *this;
                    }
      bool          replaceregexpp(const char* pzPattern, const char* pzReplacement, bool isIgnoreCase, bool replaceFirstOccurrenceOnly = false)
                    {
                      int
                        nCharsMatched;

                      const char
                        *pzWork,
                        *pzMatchFound;

                      bool
                        isFound = false;

                      pzWork = this->buffer();
                      while (true)
                      {
                        RegExp::SubStringList
                          cSubStringList;

                        if (RegExp::Search(&nCharsMatched, &pzMatchFound, pzPattern, pzWork, isIgnoreCase, &cSubStringList))
                        {
                          isFound = true;

                          Hue::Util::String
                            cHead(buffer(), (size_t)(pzMatchFound - buffer())),
                            cTail(pzMatchFound + nCharsMatched),
                            cWork;

                          const char* pzReplace = pzReplacement; 
                          while (pzReplace && *pzReplace)
                          {
                            switch (*pzReplace)
                            {
                            case '\\':
                              ++pzReplace;
                              if (*pzReplace)
                              {
                                cWork.append(*pzReplace);
                                pzReplace++;
                              }
                              break;

                            case '/':
                              if (isdigit(pzReplace[1]))
                              {
                                ++pzReplace;
                                int nSubExpression = *pzReplace & 0x0f;
                                ++pzReplace;
                                if (nSubExpression < (int)cSubStringList.size())
                                {
                                  Hue::Util::String
                                    cTemp;
                                  
                                  cTemp.assign(cSubStringList[nSubExpression]._pzBegin, cSubStringList[nSubExpression]._pzEnd);
                                  cWork.append(cTemp);
                                }
                              }
                              else
                              {
                                cWork.append(*pzReplace);
                                pzReplace++;
                              }
                              break;

                            default:
                              cWork.append(*pzReplace);
                              pzReplace++;
                              break;
                            }
                          }
                          this->assign(cHead);
                          append(cWork);
                          int nOffset = this->length();
                          append(cTail);
                          pzWork = this->buffer() + nOffset;
                          if (replaceFirstOccurrenceOnly)
                          {
                            break;
                          }
                        }
                        else
                        {
                          break;
                        }
                      }
                      return isFound;
                    }

      String&       truncate_from_first(const char * pzASCII)
                    {
                      truncate_from_firstp(pzASCII);
                      return *this;
                    }
      bool          truncate_from_firstp(const char * pzASCII)
                    {
                      char *
                        pzFound = const_cast<char*>(findfirst(pzASCII));

                      if (pzFound)
                      {
                        pzFound[0] = 0;
                        _nLength = -1;
                        _nHash = 0;
                        return true;
                      }
                      return false;
                    }

      String&       truncate_after_first(const char * pzASCII)
                    {
                      truncate_after_firstp(pzASCII);
                      return *this;
                    }
      bool          truncate_after_firstp(const char * pzASCII)
                    {
                      char *
                        pzFound = const_cast<char*>(findfirst(pzASCII));

                      if (pzFound)
                      {
                        pzFound[strlen(pzASCII)] = 0;
                        _nLength = -1;
                        _nHash = 0;
                        return true;
                      }
                      return false;
                    }

      String&       truncate_after_last(const char * pzASCII)
                    {
                      truncate_after_lastp(pzASCII);
                      return *this;
                    }
      bool          truncate_after_lastp(const char * pzASCII)
                    {
                      char* pzLast = const_cast<char*>(findlast(pzASCII));
                      if (pzLast)
                      {
                        pzLast[strlen(pzASCII)] = 0;
                        _nLength = -1;
                        _nHash = 0;
                        return true;
                      }
                      return false;
                    }

        String&     truncate_from_last(const char * pzASCII)
                    {
                      truncate_from_lastp(pzASCII);
                      return *this;
                    }
        bool        truncate_from_lastp(const char * pzASCII)
                    {
                      char* pzLast = const_cast<char*>(findlast(pzASCII));
                      if (pzLast)
                      {
                        pzLast[0] = 0;
                        _nLength = -1;
                        _nHash = 0;
                        return true;
                      }
                      return false;
                    }

      String&       truncate_trailing(const char * pzASCII)
                    {
                      truncate_trailingp(pzASCII);
                      return *this;
                    }
      bool          truncate_trailingp(const char * pzASCII)
                    {
                      int
                        nLength = length();

                      if (nLength < (int)strlen(pzASCII))
                      {
                        return false;
                      }

                      char *
                        pzLast = buffer() + length() - strlen(pzASCII);

                      if (!HUE_STRCMP(pzLast, pzASCII))
                      {
                        pzLast[0] = 0;
                        _nLength = -1;
                        _nHash = 0;
                        return true;
                      }
                      return false;
                    }

      String&       truncate_leading(const char * pzASCII)
                    {
                      truncate_leadingp(pzASCII);
                      return *this;
                    }
      bool          truncate_leadingp(const char * pzASCII)
                    {
                      int
                        nLength = length(),
                        nCompareLength = (int)strlen(pzASCII);

                      if (nLength < nCompareLength)
                      {
                        return false;
                      }
                      for (int iScan = 0; iScan < nCompareLength; iScan++)
                      {
                        if (_pzASCII[iScan] != pzASCII[iScan])
                        {
                          return false;
                        }
                      }
                      int i;
                      for (i = 0; _pzASCII[nCompareLength + i] != '\0'; ++i)
                      {
                        _pzASCII[i] = _pzASCII[nCompareLength + i];
                      }
                      _pzASCII[i] = '\0';
                      _nLength = -1;
                      _nHash = 0;
                      return true;
                    }

      String&       truncate_up_to_including_first(const char * pzASCII)
                    {
                      truncate_up_to_including_firstp(pzASCII);
                      return *this;
                    }
      bool          truncate_up_to_including_firstp(const char * pzASCII)
                    {
                      const char* pzFirst = findfirst(pzASCII);
                      if (pzFirst)
                      {
                        String cTemp(pzFirst + strlen(pzASCII));
                        assign(cTemp);
                        return true;
                      }
                      return false;
                    }

      String&       truncate_up_to_including_last(const char * pzASCII)
                    {
                      truncate_up_to_including_lastp(pzASCII);
                      return *this;
                    }
      bool          truncate_up_to_including_lastp(const char * pzASCII)
                    {
                      const char* pzLast = findlast(pzASCII);
                      if (pzLast)
                      {
                        String cTemp(pzLast + strlen(pzASCII));
                        assign(cTemp);
                        return true;
                      }
                      return false;
                    }

      String&       trim()
                    {
                      trimp();
                      return *this;
                    }
      bool          trimp()
                    {
                      bool changed = false;
                      int len = length();
                      while (truncate_trailing(" ").truncate_trailing("\t").truncate_trailing("\n").truncate_trailing("\r").length() != len)
                      {
                        len = length();
                        changed = true;
                      }
                      while (truncate_leading(" ").truncate_leading("\t").truncate_leading("\n").truncate_leading("\r").length() != len)
                      {
                        len = length();
                        changed = true;
                      }
                      return true;
                    }

      String&       limit_length(int max_length)
                    {
                      limit_lengthp(max_length);
                      return *this;
                    }
      bool          limit_lengthp(int max_length)
                    {
                      if (length() > max_length)
                      {
                        _pzASCII[max_length] = '\0';
                        return true;
                      }
                      return false;
                    }
      /* <-- conditional modification functions */

      String&       capitalize()
                    {
                      for (int i = 0; i < length(); ++i)
                      {
                        if (!::isspace(_pzASCII[i]))
                        {
                          _pzASCII[i] = ::toupper(_pzASCII[i]);
                          break;
                        }
                      }
                      return *this;
                    }

      String&       toupper()
                    {
                      for (int i = 0; i < this->length(); ++i)
                      {
                        _pzASCII[i] = ::toupper(_pzASCII[i]);
                      }
                      return *this;
                    }

      String&       tolower()
                    {
                      for (int i = 0; i < this->length(); ++i)
                      {
                        _pzASCII[i] = ::tolower(_pzASCII[i]);
                      }
                      return *this;
                    }

      String&       prepend(const char * pzASCII)
                    {
                      if (pzASCII && *pzASCII)
                      {
                        String cTemp;
                        cTemp.reallocate(length() + strlen(pzASCII), false);
                        cTemp.append(pzASCII);
                        cTemp.append(_pzASCII);
                        assign(cTemp);
                      }
                      return *this;
                    }

      String&       prepend(const String& other)
                    {
                      return prepend(other.c_str());
                    }

      String&       prepend(char c)
                    {
                      char aTemp[2];
                      aTemp[0] = c;
                      aTemp[1] = '\0';
                      this->prepend(aTemp);
                      return *this;
                    }

      String&       append(const char* pzString, size_t nLength)
                    {
                      reallocate(length() + nLength, true);
                      strncpy(_pzASCII + length(), pzString, nLength);
                      _nLength += (int)nLength;
                      _pzASCII[_nLength] = '\0';
                      _nHash = 0;
                      return *this;
                    }

      String&       append_n(const char* pzString, int count)
                    {
                      while (count--)
                      {
                        append(pzString);
                      }
                      return *this;
                    }

      String&       append_n(Hue::Util::String const& str, int count)
                    {
                      return append_n(str.c_str(), count);
                    }

      String&       append(const char* pzString)
                    {
                      if (pzString)
                      {
                        return append(pzString, strlen(pzString));
                      }
                      return *this;
                    }

      String&       append(const std::string& str)
                    {
                      return append(str.c_str());
                    }

      String&       append(const String& String)
                    {
                      return append(String.c_str());
                    }

      String&       append(const char* pzSubstringStart, const char* pzSubstringEnd)
                    {
                      if (pzSubstringStart < pzSubstringEnd)
                      {
                        return append(pzSubstringStart, (size_t)(pzSubstringEnd - pzSubstringStart));
                      }
                      return *this;
                    }

      String&       append(char c)
                    {
                      char aTemp[2];
                      aTemp[0] = c;
                      aTemp[1] = '\0';
                      this->append(aTemp);
                      return *this;
                    }

      String&       append(int number)
                    {
                      char aTemp[32];
                      sprintf(aTemp, "%d", number);
                      this->append(aTemp);
                      return *this;
                    }

      String&       center(int field_width) {
                      while (length() < field_width) {
                        if (length() & 1) {
                          append(' ');
                        } else {
                          prepend(' ');
                        }
                      }
                      return *this;
                    }

      String&       left_justify(int field_width) {
                      while (length() < field_width) {
                        append(' ');
                      }
                      return *this;
                    }

      String&       right_justify(int field_width) {
                      while (length() < field_width) {
                        prepend(' ');
                      }
                      return *this;
                    }

      String&       vsprintf(const char* format, va_list args_org) 
                    {
                      char tmp[128];
                      char* str = tmp;
                      char* alloced = 0;
                      int size = sizeof(tmp);
                      int r  = 0;
                      va_list args;
                      do {
                        va_copy(args, args_org);
                        r = vsnprintf(str, size, format, args);
                        if (r >= 0 && r < (int)size) {
                          assign(str);
                          break;
                        }
                        if (alloced) {
                          delete [] alloced;
                          if (r > size)
                            size = r + 1;
                          else
                            size *= 2;
                        } else {
                          size = 256;
                        }
                        if (size < 0)
                          return *this;
                        alloced = str = new char[size];
                      } while (true);
                      if (alloced)
                        delete [] alloced;
                      return *this;
                    }

      String&       printf(const char* format, ...) 
                    {
                       va_list args;
                       va_start(args, format);
                       vsprintf(format, args);
                       va_end(args);
                       return *this;
                    }

      static String  static_printf(const char* format, ...) 
                     {
                       String result;
                       va_list args;
                       va_start(args, format);
                       result.vsprintf(format, args);
                       va_end(args);
                       return result;
                      }

      String&       appendf(const char* format, ...) 
                    {
                       Hue::Util::String tmp;
                       va_list args;
                       va_start(args, format);
                       tmp.vsprintf(format, args);
                       append(tmp);
                       va_end(args);
                       return *this;
                    }

      String&       appendHex(int number, int field_width)
                    {
                      char aFormat[16];
                      sprintf(aFormat, "%%0%dX", field_width);
                      appendf(aFormat, number);
                      return *this;
                    }

      char const&   at(int iChar) const 
                    {
                      static char
                        nullchar = 0;

                      if (iChar < 0 || iChar >= length())
                      {
                        return nullchar;
                      }
                      return c_str()[iChar];
                    }

      char&         at(int iChar)
                    {
                      static char
                        nullchar = 0;

                      if (iChar < 0 || iChar >= length())
                      {
                        return nullchar;
                      }
                      return *(char*)(c_str() + iChar);
                    }

      bool          starts_with(const char* pzASCII) const
                    {
                      return static_starts_with(_pzASCII, pzASCII);
                    }

      static bool   static_starts_with(const char* pzString, const char* pzStartsWith)
                    {
                      if (pzString && pzStartsWith)
                      {
                        if (strncmp(pzString, pzStartsWith, strlen(pzStartsWith)) == 0)
                        {
                          return true;
                        }
                      }
                      return false;
                    }

      bool          ends_with(const char* pzASCII) const
                    {
                      return static_ends_with(_pzASCII, pzASCII);
                    }

      static bool   static_ends_with(const char* pzString, const char* pzEndsWith)
                    {
                      if (pzString && pzEndsWith)
                      {
                        if (strlen(pzEndsWith) <= strlen(pzString))
                        {
                          size_t nOffset = strlen(pzString) - strlen(pzEndsWith);
                          if (HUE_STRCMP(pzString + nOffset, pzEndsWith) == 0)
                          {
                            return true;
                          }
                        }
                      }
                      return false;
                    }

      List          split(const char* delimiters, bool includedelimiters = false, bool trim_leading_and_trailing_whitespace = false, bool include_empty_tokens = true) const
                    {
                      List
                        list;

                      String
                        sWork;

                      bool
                        isDelim = false;

                      for (int i = 0; i < this->length(); ++i)
                      {
                        if (strchr(delimiters, at(i)))
                        {
                          if (trim_leading_and_trailing_whitespace)
                          {
                            sWork.trim();
                          }
                          if (include_empty_tokens || sWork.length())
                          {
                            list.append(sWork);
                            sWork.clear();
                          }
                          if (includedelimiters)
                          {
                            sWork.append(at(i));
                            list.append(sWork);
                            sWork.clear();
                          }
                          isDelim = true;
                        }
                        else
                        {
                          isDelim = false;
                          sWork.append(at(i));
                        }
                      }
                      if (sWork.length())
                      {
                        list.append(sWork);
                      }
                      else if (isDelim && include_empty_tokens)
                      {
                        list.append(sWork);
                      }
                      return list;
                    }

      static List   split(const char* str, const char* delimiters, bool includedelimiters = false, bool trim_leading_and_trailing_whitespace = false, bool include_empty_tokens = true) 
                    {
                      String 
                        sString(str);

                      return sString.split(delimiters, includedelimiters, trim_leading_and_trailing_whitespace, include_empty_tokens);
                    }

      String&       insert(const String& str, int insert_pos)
                    {
                      String
                        head(this->c_str(), insert_pos),
                        tail(this->c_str() + insert_pos);

                      this->assign(head);
                      this->append(str);
                      this->append(tail);
                      return *this;
                    }

      String&       limit_line_length(int max_len, const char* newline = "\n")
                    {
                      Hue::Util::String::List lines = this->split(newline, true, false, false);
                      Hue::Util::String::List new_lines;
                      for (int i = 0; i < lines.count(); ++i)
                      {
                        if (lines[i].length() <= max_len)
                        {
                          new_lines.push_back(lines[i]);
                        }
                        else
                        {
                          Hue::Util::String prefix = "";
                          Hue::Util::String::List words = lines[i].split(" ", false, true);
                          int nonemptyword = 0;
                          while (words.size() >= nonemptyword && words[nonemptyword].empty())
                          {
                            ++nonemptyword;
                          }
                          if (words.size())
                          {
                            const char* prefix_end = lines[i].findfirst(words[nonemptyword].c_str());
                            prefix.assign(lines[i].c_str(), prefix_end);
                          }
                          Hue::Util::String current_line;
                          for (int w = 0; w < words.count(); ++w)
                          {
                            if (current_line.length() + words[w].length() + 1 > max_len)
                            {
                              current_line.append(newline);
                              new_lines.push_back(current_line);
                              current_line = prefix;
                              current_line.append(words[w]);
                            }
                            else
                            {
                              if (!words[w].empty())
                              {
                                if (current_line.empty())
                                {
                                  current_line = prefix;
                                }
                                else
                                {
                                  current_line.append(" ");
                                }
                                current_line.append(words[w]);
                              }
                            }
                          }
                          if (current_line.length())
                          {
                            new_lines.push_back(current_line);
                          }
                        }
                      }
                      Hue::Util::String new_str;
                      for (int i = 0; i < new_lines.count(); ++i)
                      {
                        new_str.append(new_lines[i]);
                      }
                      assign(new_str);
                      return *this;
                    }

      static String limit_line_length(const char* text, int max_len, const char* newline = "\n")
                    {
                      String work(text);
                      work.limit_line_length(max_len, newline);
                      return work;
                    }

      bool operator== (const char* other) const
                    {
                      return equal(other);
                    }

      bool operator!= (const char* other) const
                    {
                      return !equal(other);
                    }

      bool operator== (const String& other) const
                    {
                      return equal(other);
                    }

      bool operator!= (const String& other) const
                    {
                      return !equal(other);
                    }

      String& operator= (const char* str)
                    {
                      assign(str);
                      return *this;
                    }

      String& operator= (const String& String)
                    {
                      assign(String);
                      return *this;
                    }

      String& operator= (const std::string& str)
                    {
                      assign(str);
                      return *this;
                    }

      bool operator< (const String& rhs) const
                    {
                      return HUE_STRCMP(this->c_str(), rhs.c_str()) < 0;
                    }

      String& operator << (const char* pzString)
                    {
                      append(pzString);
                      return *this;
                    }

      String& operator << (const String& other)
                    {
                      append(other);
                      return *this;
                    }

      String& operator << (const std::string& other)
                    {
                      append(other);
                      return *this;
                    }

      String& operator << (const int& rhs)
                    {
                      this->append(rhs);
                      return *this;
                    }

      String& operator += (const char* pzString)
                    {
                      return append(pzString);
                    }

      String& operator += (const String& other)
                    {
                      return append(other);
                    }

      String& operator += (const std::string& other)
                    {
                      return append(other);
                    }

      String& operator += (const int& rhs)
                    {
                      return this->append(rhs);
                    }

      operator const char* () const
                    {
                      return c_str();
                    }

      bool empty() const { return length() == 0; }

      static void unittest();

      struct LessThanPredicate
      {
        struct IgnoreCase
        {
          bool operator() (const String& lhs, const String& rhs) const
          {
            return HUE_STRICMP(lhs.c_str(), rhs.c_str()) < 0;
          }
        };

        struct CaseSensitive
        {
          bool operator() (const String& lhs, const String& rhs) const
          {
            return HUE_STRCMP(lhs.c_str(), rhs.c_str()) < 0;
          }
        };
      };

    private:
      char *        _pzASCII;

      char *        _pzALLOCATED;

      char          _azTemp[100];

      size_t        _nSize;

      mutable int   _nLength;

      mutable Int64
                    _nHash;

    };

    String operator+ (String const& lhs, String const& rhs);
    String operator+ (String const& lhs, const char* rhs);
    String operator+ (const char* lhs, String const& rhs);

  } // namespace Util
} // namespace Hue

#endif // !defined HUE_UTIL_STRING_H_INCLUDED
