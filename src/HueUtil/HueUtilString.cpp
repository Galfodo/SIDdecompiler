#include "HueUtilString.h"
#include <assert.h>
#include <stdlib.h>
#include <map>
//#include <unordered_map>
//#include <hash_map>

#define REGISTER_PATTERNS 0 // if non-zero, a map of patterns with skip tables is maintained

namespace Hue
{
  namespace Util
  {
    template<>
    Int64         stringHashT<false>(const char* pzString, size_t length)
                  {
                    Int64
                      nHash            = 0,
                      nMultiplier0     = 0x78f1239a0bcde456LL,
                      nMultiplier1     = 0x9abd23015ef6784cLL,
                      nMultiplierDelta = 0xb256d09f3c78ae41LL;

                    for (int iCount = 0; iCount < length; iCount++)
                    {
                      char
                        zASCII = pzString[iCount];

                      nHash += zASCII * nMultiplier0;

                      nMultiplier0 += nMultiplier1;
                      nMultiplier0 ^= 0xa37d4ef12589bc60LL;

                      nMultiplier1 += nMultiplier0 * nMultiplierDelta;
                      nMultiplier1 ^= nMultiplier0 >> 23;

                      nMultiplierDelta += 0x09c761bde345a28fLL;
                      nMultiplierDelta ^= 0x5def340178abc629LL;
                    }
                    return nHash;
                  }

    template<>
    Int64         stringHashT<true>(const char* pzString, size_t length)
                  {
                    Int64
                      nHash            = 0,
                      nMultiplier0     = 0x78f1239a0bcde456LL,
                      nMultiplier1     = 0x9abd23015ef6784cLL,
                      nMultiplierDelta = 0xb256d09f3c78ae41LL;

                    for (int iCount = 0; iCount < length; iCount++)
                    {
                      char
                        zASCII = ::toupper(pzString[iCount]);

                      nHash += zASCII * nMultiplier0;

                      nMultiplier0 += nMultiplier1;
                      nMultiplier0 ^= 0xa37d4ef12589bc60LL;

                      nMultiplier1 += nMultiplier0 * nMultiplierDelta;
                      nMultiplier1 ^= nMultiplier0 >> 23;

                      nMultiplierDelta += 0x09c761bde345a28fLL;
                      nMultiplierDelta ^= 0x5def340178abc629LL;
                    }
                    return nHash;
                  }


#if HUE_USE_FAST_STRING_SEARCH

    struct PatternRecord;
    typedef std::map<Int64, PatternRecord*> PatternMap_t;
    typedef Int64 skipcount_t;

    struct PatternRecord 
    {
      void init(const char* pattern, size_t pattern_length, bool isCaseInsensitive)
      {
        this->m_PatternLength = pattern_length;
        this->m_CaseInsensitive = isCaseInsensitive;
#if REGISTER_PATTERNS
        if (isCaseInsensitive)
        {
          for (size_t i = 0; i < pattern_length; ++i)
          {
            this->m_Pattern[i] = toupper(pattern[i]);
          }
        }
        else
        {
          memcpy(this->m_Pattern, pattern, pattern_length);
        }
        this->m_Pattern[pattern_length] = '\0';
#else
        this->m_Pattern = pattern;
#endif
        const unsigned char* pat = (const unsigned char*)this->m_Pattern;
        for (size_t i = 0; i < 256; ++i) {
          this->m_SkipTable[i] = (skipcount_t)pattern_length;
        }
        for (size_t i = 0; i < pattern_length; ++i) {
          this->m_SkipTable[pat[i]] = (skipcount_t)(pattern_length - i - 1);
        }
      }

      static PatternRecord* Create(const char* pattern, size_t pattern_length, bool isCaseInsensitive) 
      {
        PatternRecord* rec = (PatternRecord*)malloc(sizeof(PatternRecord) + pattern_length);
        rec->init(pattern, pattern_length, isCaseInsensitive);
        return rec;
      }

      skipcount_t m_SkipTable[256];
      size_t      m_PatternLength;
      bool        m_CaseInsensitive;
#if REGISTER_PATTERNS
      char        m_Pattern[1];
#else
      const char* m_Pattern;
#endif
    };

#if REGISTER_PATTERNS
    static PatternMap_t
      s_PatternRecordMap;
#endif

    class StringSearcher {
    public:
      StringSearcher(const char* pattern, size_t pattern_length, bool caseInsensitive = false) : m_PatternRecord(NULL) 
      {
        if (caseInsensitive)
        {
          init<true>(pattern, pattern_length);
        }
        else
        {
          init<false>(pattern, pattern_length);
        }
      }

      ~StringSearcher()
      {
#if !REGISTER_PATTERNS
        m_PatternRecord = NULL;
#endif
      }

      template<bool ISCASEINSENSITIVE>
      void init(const char* pattern, size_t pattern_length) {
        assert(m_PatternRecord == NULL);
#if REGISTER_PATTERNS
        Int64 hash = ISCASEINSENSITIVE ? Hue::Util::stringHashT<true>(pattern, pattern_length) : Hue::Util::stringHashT<false>(pattern, pattern_length);
        PatternMap_t::iterator it = s_PatternRecordMap.find(hash);
        PatternRecord* rec = NULL;
        if (it == s_PatternRecordMap.end())
        {
          rec = PatternRecord::Create(pattern, pattern_length, ISCASEINSENSITIVE);
          s_PatternRecordMap.insert(std::make_pair(hash, rec));
        }
        else
        {
          rec = it->second;
        }
        assert(rec);
        m_PatternRecord = rec;
#else
        m_Rec_.init(pattern, pattern_length, ISCASEINSENSITIVE);
        m_PatternRecord = &m_Rec_;
#endif
      }
      
      // Fast string search. Returns the number of characters to skip forward in the search_buffer 
      // for each attempt that is made.
      template<bool ISCASESENSITIVE>
      bool match(skipcount_t& skip, const char* search_buffer) const;

      

      // Utility functions //////////////////////////////////////////////////

      template<bool CASEINSENSITIVE>
      const char* find_next(const char* buffer, size_t remaining) const;      

      template<bool ISCASEINSENSITIVE>
      static const char* find_first(const char* pattern, size_t pattern_length, const char* buffer, size_t buffer_length) {
        StringSearcher
          searcher(pattern, pattern_length, ISCASEINSENSITIVE);

        return searcher.find_next<ISCASEINSENSITIVE>(buffer, buffer_length);
      }

      template<bool ISCASEINSENSITIVE>
      static const char* find_last(const char* pattern, size_t pattern_length, const char* buffer, size_t buffer_length) {
        StringSearcher
          searcher(pattern, pattern_length, ISCASEINSENSITIVE);

        skipcount_t skip = 0;
        const char* lastfound = searcher.find_next<ISCASEINSENSITIVE>(buffer, buffer_length);
        const char* found = lastfound;
        while (found) {
          size_t skip = (found - buffer) + pattern_length;
          buffer += skip;
          buffer_length -= skip;
          found = searcher.find_next<ISCASEINSENSITIVE>(buffer, buffer_length);
          if (found) {
            lastfound = found;
          }
        }
        return lastfound;
      }

    private:
      PatternRecord* m_PatternRecord;
#if !REGISTER_PATTERNS
      PatternRecord m_Rec_;
#endif
    }; // StringSearcher

      template<>
      bool 
      StringSearcher::match<false>(skipcount_t& skip, const char* search_buffer) const {
        assert(m_PatternRecord);
        assert(!m_PatternRecord->m_CaseInsensitive);
        if (m_PatternRecord->m_PatternLength == 0) {
          skip = 1;
          return true;
        }
        unsigned char const* work = (unsigned char const*)search_buffer + m_PatternRecord->m_PatternLength - 1;
        unsigned char const* pattern = (unsigned char const*)m_PatternRecord->m_Pattern + m_PatternRecord->m_PatternLength - 1;
        for (size_t i = 0; i < m_PatternRecord->m_PatternLength; ++i) {
          if (*work != *pattern) {
            skip = m_PatternRecord->m_SkipTable[*work] - (skipcount_t)i;
            if (skip < 1) {
              skip = 1;
            }
            return false;
          }
          --work;
          --pattern;
        }
        skip = (skipcount_t)m_PatternRecord->m_PatternLength;
        return true;
      }

      template<>
      bool 
      StringSearcher::match<true>(skipcount_t& skip, const char* search_buffer) const {
        assert(m_PatternRecord);
        assert(m_PatternRecord->m_CaseInsensitive);
        if (m_PatternRecord->m_PatternLength == 0) {
          skip = 1;
          return true;
        }
        unsigned char const* work = (unsigned char const*)search_buffer + m_PatternRecord->m_PatternLength - 1;
        unsigned char const* pattern = (unsigned char const*)m_PatternRecord->m_Pattern + m_PatternRecord->m_PatternLength - 1;
        for (size_t i = 0; i < m_PatternRecord->m_PatternLength; ++i) {
          int c = toupper(*work);
          if (c != *pattern) { // pattern is already upper case
            skip = (skipcount_t)(m_PatternRecord->m_SkipTable[c] - i);
            if (skip < 1) {
              skip = 1;
            }
            return false;
          }
          --work;
          --pattern;
        }
        skip = (skipcount_t)m_PatternRecord->m_PatternLength;
        return true;
      }
      
      template<>
      const char* 
      StringSearcher::find_next<true>(const char* buffer, size_t remaining) const {
        assert(m_PatternRecord);
        assert(buffer);
        assert(m_PatternRecord->m_CaseInsensitive);
        if (m_PatternRecord->m_PatternLength <= remaining) {
          for (const char* work = buffer; remaining; ) {
            skipcount_t skip = 0;
            if (match<true>(skip, work)) {
              return work;
            } else {
              if (skip > (skipcount_t)remaining) {
                skip = (skipcount_t)remaining;
              } 
            }
            assert(skip > 0);
            assert(skip <= (skipcount_t)m_PatternRecord->m_PatternLength);
            assert((size_t)skip <= remaining);
            work += skip;
            remaining -= skip;
          }
          assert(remaining == 0);
          return NULL;
        } else {
          return NULL;
        }
      }

      template<>
      const char* 
      StringSearcher::find_next<false>(const char* buffer, size_t remaining) const {
        assert(m_PatternRecord);
        assert(buffer);
        assert(!m_PatternRecord->m_CaseInsensitive);
        if (m_PatternRecord->m_PatternLength <= remaining) {
          for (const char* work = buffer; remaining; ) {
            skipcount_t skip = 0;
            if (match<false>(skip, work)) {
              return work;
            } else {
              if (skip > (skipcount_t)remaining) {
                skip = (skipcount_t)remaining;
              } 
            }
            assert(skip > 0);
            assert(skip <= (skipcount_t)m_PatternRecord->m_PatternLength);
            assert((size_t)skip <= remaining);
            work += skip;
            remaining -= skip;
          }
          assert(remaining == 0);
          return NULL;
        } else {
          return NULL;
        }
      }


    const char*   
    Hue::Util::String::fast_findfirst(const char* pzSearchBuffer, size_t nSearchBufferLen, const char* pzPattern, size_t nPatternLen)
    {
      assert(pzSearchBuffer);
      assert(pzPattern);
      return StringSearcher::find_first<false>(pzPattern, nPatternLen, pzSearchBuffer, nSearchBufferLen);
    }

    const char*   
    Hue::Util::String::fast_findfirstcaseinsensitive(const char* pzSearchBuffer, size_t nSearchBufferLen, const char* pzPattern, size_t nPatternLen)
    {
      assert(pzSearchBuffer);
      assert(pzPattern);
#if !REGISTER_PATTERNS
      Hue::Util::String
        sPattern(pzPattern, nPatternLen);

      sPattern.toupper();
      pzPattern = sPattern.c_str();
#endif
      return StringSearcher::find_first<true>(pzPattern, nPatternLen, pzSearchBuffer, nSearchBufferLen);
    }

    const char* 
    Hue::Util::String::fast_findlast(const char* pzSearchBuffer, size_t nSearchBufferLen, const char* pzPattern, size_t nPatternLen)
    {
      assert(pzPattern);
      return StringSearcher::find_last<false>(pzPattern, nPatternLen, pzSearchBuffer, nSearchBufferLen);
    }

    // NB! pzNewBuffer must be NULL or large enough to hold the new string
    // if pzNewBuffer is NULL, the function merely counts the number of replacements that will be made
    // RETURNS: Number of replacements
    // NB! This function 0-terminates the new buffer
    int    
    Hue::Util::String::fast_replace(char* pzNewBuffer, const char* pzSearchBuffer, size_t nSearchBufferLen, const char * pzNewPattern, size_t nNewPatternLen, const char * pzOldPattern, size_t nOldPatternLen)
    {
      StringSearcher
        searcher(pzOldPattern, nOldPatternLen, false);

      int replacements = 0;
      if (pzNewBuffer == NULL) 
      {
        skipcount_t skip = 0;

        const char
          *pzFound = searcher.find_next<false>(pzSearchBuffer, nSearchBufferLen);

        while (pzFound)
        {
          ++replacements;
          size_t skip = (pzFound - pzSearchBuffer) + nOldPatternLen;
          
          pzSearchBuffer += skip;
          nSearchBufferLen -= skip;
          pzFound = searcher.find_next<false>(pzSearchBuffer, nSearchBufferLen);
        }
        return replacements;
      }
      else
      {
        size_t
          nWritten = 0;

        skipcount_t skip = 0;

        char
          *pzWork = pzNewBuffer;

        const char
          *pzFound = searcher.find_next<false>(pzSearchBuffer, nSearchBufferLen);

        while (pzFound)
        {
          size_t diff = (size_t)(pzFound - pzSearchBuffer);
          size_t skip = diff + nOldPatternLen;
          memcpy(pzWork, pzSearchBuffer, diff);
          pzWork += diff;
          nWritten += diff;
          memcpy(pzWork, pzNewPattern, nNewPatternLen);
          pzWork += nNewPatternLen;
          nWritten += nNewPatternLen;

          ++replacements;
          pzSearchBuffer += skip;
          nSearchBufferLen -= skip;
          pzFound = searcher.find_next<false>(pzSearchBuffer, nSearchBufferLen);
        }
        memcpy(pzWork, pzSearchBuffer, nSearchBufferLen);
        pzWork += nSearchBufferLen;
        nWritten += nSearchBufferLen;
        *pzWork = '\0';
        return replacements;
      }
    } // clas Hue::Util::String      
#endif

    String operator+ (String const& lhs, String const& rhs) 
    {
      String
        newString(lhs);

      newString.append(rhs);
      return newString;
    }

    String operator+ (String const& lhs, const char* rhs) 
    {
      String
        newString(lhs);

      newString.append(rhs);
      return newString;
    }

    String operator+ (const char* lhs, String const& rhs)
    {
      String
        newString(lhs);

      newString.append(rhs);
      return newString;
    }

    void Hue::Util::String::unittest()
    {
      Hue::Util::String::List list = Hue::Util::String::split("\n\n", "\n", true, false, false);
      assert(list.count() == 2);
      list = Hue::Util::String::split(",,,", ",", false, true, true);
      assert(list.count() == 4);
    }


 }
}
