#ifndef HUE_UTIL_REGEXP_H_INCLUDED
#define HUE_UTIL_REGEXP_H_INCLUDED

#include <cstring>
#include <vector>

// CLASSES //////////////////////////////////////////////////////////////////

namespace Hue 
{
  namespace Util
  {
    class RegExp
    {
    public:
      // This class represents a substring of the matched string
      class SubString
      {
      public:
        const char* _pzBegin;
        const char* _pzEnd;

        SubString() : _pzBegin(0), _pzEnd(0)
        {
        }
      };

      // List of matched substrings
      class SubStringList
      {
      public:
        SubString& operator [] (int iElement)
        {
          while (iElement >= (int)_lcSubString.size())
          {
            SubString cEmpty;
            _lcSubString.push_back(cEmpty);
          }
          return _lcSubString[iElement];
        }

        size_t size() const
        {
          return _lcSubString.size();
        }

      private:
        std::vector<SubString>
          _lcSubString;
      };

      // Context for pattern matcher
      class REContext;

      // Base class for regexp
      class REBase;

      /// Compile regular expression
      static REBase*  CompileRegExp(const char* pzPattern);

      /// Compile simple wildcard (* and ? characters) expression
      static REBase*  CompileWildcardExp(const char* pzPattern);

      /// See if regular expression matches the supplied string
      static bool     Match(int* pnCharactersMatched, REBase* pREBase, const char* pzString, bool isIgnoreCase = false, SubStringList* pcSubStringList = 0);

      /// See if regular expression matches the supplied string
      static bool     Match(int* pnCharactersMatched, const char* pzPattern, const char* pzString, bool isIgnoreCase = false, SubStringList* pcSubStringList = 0);

      /// See if wildcard (* and ? characters) expression matches the supplied string
      static bool     MatchWildcard(int* pnCharactersMatched, const char* pzPattern, const char* pzString, bool isIgnoreCase = false);

      /// Find the longes matching substring
      static bool     Search(int* pnCharactersMatched, const char** ppzMatchFound, REBase* pREBase, const char* pzString, bool isIgnoreCase = false, SubStringList* pcSubStringList = 0);

      /// Find the longes matching substring
      static bool     Search(int* pnCharactersMatched, const char** ppzMatchFound, const char* pzPattern, const char* pzString, bool isIgnoreCase = false, SubStringList* pcSubStringList = 0);
    };

  } // namespace Util

} // namespace Hue

#endif // !defined
