/***************************************************************************\

  NAME:         REGEXP.CPP

  AUTHOR:       stein@hue.no

  DESCRIPTION:  Provides simple regular expression pattern matching. 
                (ripped from HueSpace)

  Regexp pattern special characters:

  .       Matches any character except null.
  ?       Matches the preceding expression zero or one times
  *       Matches the preceding expression zero or more times
  +       Matches the preceding expression one or more times
  \n      Matches newline character
  \r      Matches carriage return character
  \t      Matches tab character
  \s      Matches any whitespace caracter, shorthand for "[ \t\r\n]"
  \S      Matches any non-whitespace character, shorthand for "[^ \t\r\n]"
  \i      Matches any c++ identifier, shorthand for "[_a-zA-Z][_0-9a-zA-Z]*"
  \'char' Backslash is used to escape a character, e.g "\*" matches the * character.
  [...]   Matches any character in the set [...], e.g the pattern "[ \t\r\n]" matches any whitespace character
  [^...]  Matches any character NOT in the set [...], e.g the pattern "[^ \t\r\n]" matches any non-whitespace character
  ()      Tag substring in match, e.g the pattern ".*([a-zA-Z])+[ \t\n\r]*\\(" will match the string "int printf(const char*, ...);" and tag the substring "printf"

  Wildcard pattern special characters:

  ?       Matches any character except null.
  *       Matches any character zero or more times.

\***************************************************************************/

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include "RegExp.h"

#include <ctype.h>
#include <stack>
#include <limits.h>

namespace Hue {
  namespace Util {

// Context of regexp matcher
class RegExp::REContext
{
public:

  REContext(bool isIgnoreCase)
  {
    _nCharactersMatched = 0;
    _pcSubStringList = 0;
    _isIgnoreCase = isIgnoreCase;
  }

  REContext(const REContext& cOther)
  {
    _nCharactersMatched = 0;
    _pcSubStringList = cOther._pcSubStringList;
    _isIgnoreCase = cOther._isIgnoreCase;
  }

  int 
    _nCharactersMatched;

  bool
    _isIgnoreCase;

  SubStringList
    *_pcSubStringList;
};

class RegExp::REBase
{
public:
  REBase* _pcNext;

                REBase();
  virtual       ~REBase();
  bool          MatchNext(REContext* pcContext, const char* pzString);
  bool          IsEqualChar(int nChar1, int nChar2, bool isIgnoreCase);
  virtual bool  Match(REContext* pcContext, const char* pzString) = 0;
};

/////////////////////////////////////////////////////////////////////////////
// REBase::REBase

RegExp::REBase::REBase()
{
  _pcNext = 0;
}

/////////////////////////////////////////////////////////////////////////////
// REBase::~REBase

RegExp::REBase::~REBase()
{
  delete _pcNext;
}

/////////////////////////////////////////////////////////////////////////////
// REBase::IsEqualChar

bool
RegExp::REBase::IsEqualChar(int nChar1, int nChar2, bool isIgnoreCase)
{
  if (isIgnoreCase)
  {
    return toupper(nChar1) == toupper(nChar2);
  }
  return nChar1 == nChar2;
}

/////////////////////////////////////////////////////////////////////////////
// REBase::MatchNext
// See if rest of string matches
bool 
RegExp::REBase::MatchNext(RegExp::REContext* pcContext, const char* pzString)
{
  if (_pcNext)
  {
    REContext
      cLocalContext(*pcContext);

    if (_pcNext->Match(&cLocalContext, pzString))
    {
      pcContext->_nCharactersMatched += cLocalContext._nCharactersMatched;
      return true;
    }
    else
    {
      pcContext->_nCharactersMatched = 0;
      return false;
    }
  }
  return true;
}

// class REChar:
// Matches a specific character
class REChar : public RegExp::REBase
{
public:
  bool Match(RegExp::REContext* pcContext, const char* pzString)
  {
    pcContext->_nCharactersMatched = 0;
    if (IsEqualChar(pzString[0], _nChar, pcContext->_isIgnoreCase))
    {
      pcContext->_nCharactersMatched++;
      return MatchNext(pcContext, pzString + 1);
    }
    return false;
  }

  REChar(int nChar)
  {
    _nChar = nChar;
  }

  int _nChar;
};

// class REAnyChar 
// Matches any character but NULL
class REAnyChar : public RegExp::REBase
{
public:
  bool Match(RegExp::REContext* pcContext, const char* pzString)
  {
    if (pzString[0])
    {
      pcContext->_nCharactersMatched++;
      return MatchNext(pcContext, pzString + 1);
    }
    return false;
  }
};

// class RESubExpression 
// Matches zero or more instances of a subexpression
class RESubExpression : public RegExp::REBase
{
public:
  bool Match(RegExp::REContext* pcContext, const char* pzString)
  {
    bool isMatch = false;
    int nMaxCharactersMatched = 0;
    int nLocalMatch = 0;
    int nMatches = 0;
    do 
    {
      RegExp::REContext cLocalContext(*pcContext);
      cLocalContext._pcSubStringList = NULL;
      if (MatchNext(&cLocalContext, pzString))
      {
        isMatch = true;
        if ((cLocalContext._nCharactersMatched + nLocalMatch) > nMaxCharactersMatched)
        {
          if (pcContext->_pcSubStringList)
          {
            cLocalContext._nCharactersMatched = 0;
            cLocalContext._pcSubStringList = pcContext->_pcSubStringList;
            MatchNext(&cLocalContext, pzString); // Perform match again to update substring tag positions
          }
          nMaxCharactersMatched = cLocalContext._nCharactersMatched + nLocalMatch;
        }
      }
      RegExp::REContext cChildContext(*pcContext);
      if (_pcChild->Match(&cChildContext, pzString))
      {
        nMatches++;
        nLocalMatch += cChildContext._nCharactersMatched;
        pzString += cChildContext._nCharactersMatched;
      }
      else
      {
        break;
      }
    } while(true);
    if (isMatch && nMatches >= _nMinMatches && nMatches <= _nMaxMatches)
    {
      pcContext->_nCharactersMatched += nMaxCharactersMatched;
      return true;
    }
    pcContext->_nCharactersMatched = 0;
    return false;
  }

  ~RESubExpression()
  {
    delete _pcChild;
  }

  RESubExpression(int nMinMatches, int nMaxMatches)
  {
    _nMinMatches = nMinMatches;
    _nMaxMatches = nMaxMatches;
    _pcChild = 0;
  }

  int     _nMinMatches,
          _nMaxMatches;
  REBase* _pcChild;
};

// class RESubBegin 
// Marks the start of a match substring
class RESubBegin : public RegExp::REBase
{
public:
  bool Match(RegExp::REContext* pcContext, const char* pzString)
  {
    if (MatchNext(pcContext, pzString))
    {
      if (pcContext->_pcSubStringList)
      {
        (*pcContext->_pcSubStringList)[_iID]._pzBegin = pzString;
      }
      return true;
    }
    return false;
  }

  RESubBegin(int iID)
  {
    _iID = iID;
  }

  int _iID;
};

// class RESubEnd 
// Marks the end of a match substring
class RESubEnd : public RegExp::REBase
{
public:
  bool Match(RegExp::REContext* pcContext, const char* pzString)
  {
    if (MatchNext(pcContext, pzString))
    {
      if (pcContext->_pcSubStringList)
      {
        (*pcContext->_pcSubStringList)[_iID]._pzEnd = pzString;
      }
      return true;
    }
    return false;
  }

  RESubEnd(int iID)
  {
    _iID = iID;
  }

  int _iID;
};

// class RECharClass
// Matches any character in a character class, OR, any character NOT in the character class
class RECharClass : public RegExp::REBase
{
public:
  bool Match(RegExp::REContext* pcContext, const char* pzString)
  {
    if (pzString[0])
    {
      if (strchr(_pzCharacters, pzString[0]) && _isMatchCharsInClass)
      {
        pcContext->_nCharactersMatched++;
        return MatchNext(pcContext, pzString + 1);
      }
      else if (!strchr(_pzCharacters, pzString[0]) && !_isMatchCharsInClass)
      {
        pcContext->_nCharactersMatched++;
        return MatchNext(pcContext, pzString + 1);
      }
    }
    pcContext->_nCharactersMatched = 0;
    return false;
  }

  RECharClass(const char* pzCharacters, bool isMatchCharsInClass)
  {
    _isMatchCharsInClass = isMatchCharsInClass;
    _pzCharacters = new char [strlen(pzCharacters) + 1];
    strcpy(_pzCharacters, pzCharacters);
  }

  ~RECharClass()
  {
    delete _pzCharacters;
  }

  // ParseCharClass
  // Parses char class. Examples:
  //  "[0123456789a-zA-Z]" : matches alphanumeric character.
  //  "[ \t\n\r]" : matches a whitespace character.
  //  "[^ \t\n\r]" : matches a non-whitespace character.
  //  "[/\\]" : matches a forward or backward slash character.
  static RECharClass*
  ParseCharClass(const char** ppzNext, const char* pzGroup)
  {
    int nChars = 0;
    char azClass[256] = "";
    bool isMatchCharsInClass = true;
    if (*pzGroup == '[')
    {
      ++pzGroup;
    }
    if (*pzGroup == '^')
    {
      isMatchCharsInClass = false;
      ++pzGroup;
    }
    for (nChars = 0; nChars < (sizeof(azClass)/sizeof(azClass[0]) - 1) && *pzGroup && *pzGroup != ']'; )
    {
      switch(*pzGroup)
      {
      case '-':
        {
          pzGroup++;
          if (nChars == 0)
          {
            return 0;
          }
          int nRangeMin = azClass[nChars - 1];
          int nRangeMax = *pzGroup;
          if (nRangeMax <= nRangeMin)
          {
            return 0;
          }
          for (;nRangeMin < nRangeMax && nChars < (sizeof(azClass)/sizeof(azClass[0]) - 1); ++nRangeMin)
          {
            azClass[nChars++] = (char)(nRangeMin + 1);
          }
        }
        break;

      case '\\':
        pzGroup++;
        switch(*pzGroup)
        {
        case 't':
          azClass[nChars] = '\t';
          break;

        case 'r':
          azClass[nChars] = '\r';
          break;

        case 'n':
          azClass[nChars] = '\n';
          break;

        default:
          azClass[nChars] = *pzGroup;
          break;
        }
        nChars++;
        break;

      default:
        azClass[nChars] = *pzGroup;
        nChars++;
        break;
      }
      ++pzGroup;
    }
    azClass[nChars] = '\0';
    if (ppzNext)
    {
      *ppzNext = pzGroup;
    }
    return new RECharClass(azClass, isMatchCharsInClass);
  }

private:
  char*
    _pzCharacters;

  bool
    _isMatchCharsInClass;
};

/////////////////////////////////////////////////////////////////////////////
// CompileWildcardExp
// Compile simple shell-type wildcard (* and ?) expression

RegExp::REBase*
RegExp::CompileWildcardExp(const char* pzPattern)
{
  std::stack<REBase*>
    patternStack;

  REBase* pNew;
  while(*pzPattern)
  {
    pNew = 0;
    switch(*pzPattern)
    {
    case '?':
      pNew = new REAnyChar;
      break;

    case '*':
      {
        REBase* pChild = new REAnyChar;
        RESubExpression* pSubExpression = new RESubExpression(0, INT_MAX);
        pSubExpression->_pcChild = pChild;
        pNew = pSubExpression;
      }
      break;

    default:
      pNew = new REChar(*pzPattern);
      break;
    }
    patternStack.push(pNew);
    pzPattern++;
  }
  REBase* pFirst = 0;
  while (!patternStack.empty())
  {
    REBase* pFront = patternStack.top();
    patternStack.pop();
    pFront->_pcNext = pFirst;
    pFirst = pFront;
  }
  return pFirst;
}

/////////////////////////////////////////////////////////////////////////////
// CompileRegExp
// Compile regular expression

RegExp::REBase*
RegExp::CompileRegExp(const char* pzPattern)
{
  std::stack<REBase*>
    patternStack;

  std::stack<RESubBegin*>
    substringStack;

  int
    iPar = 0,
    iParLevel = 0;

  REBase* pNew = NULL;
  while(*pzPattern)
  {
    pNew = 0;
    switch(*pzPattern)
    {
    case '(':
      {
        RESubBegin* pRESubBegin = new RESubBegin(iPar);
        ++iPar;
        substringStack.push(pRESubBegin);
        pNew = pRESubBegin;
      }
      break;

    case ')':
      if (substringStack.empty())
      {
        goto error;
      }
      else
      {
        RESubBegin* pRESubBegin = substringStack.top();
        substringStack.pop();
        RESubEnd* pRESubEnd = new RESubEnd(pRESubBegin->_iID);
        pNew = pRESubEnd;
      }
      break;

    case '*': case '+': case '?':
      {
        if (patternStack.empty())
        {
          goto error;
        }
        REBase* pChild = patternStack.top();
        patternStack.pop();
        RESubExpression* pSubExpression = NULL;
        switch (*pzPattern)
        {
        case '*':
          pSubExpression = new RESubExpression(0, INT_MAX);
          break;
        case '+':
          pSubExpression = new RESubExpression(1, INT_MAX);
          break;
        case '?':
          pSubExpression = new RESubExpression(0, 1);
          break;
        }
        pSubExpression->_pcChild = pChild;
        pNew = pSubExpression;
      }
      break;

    case '.':
      pNew = new REAnyChar;
      break;

    case '[':
      pNew = RECharClass::ParseCharClass(&pzPattern, pzPattern);
      if (!pNew)
      {
        goto error;
      }
      break;

    case '\\':
      pzPattern++;
      switch(*pzPattern)
      {
      case 't':
        pNew = new REChar('\t');
        break;

      case 'n':
        pNew = new REChar('\n');
        break;

      case 'r':
        pNew = new REChar('\r');
        break;

      case 'i':
        { // C++ identifier
          pNew = RECharClass::ParseCharClass(NULL, "[_a-zA-Z]");
          patternStack.push(pNew);
          pNew = RECharClass::ParseCharClass(NULL, "[_a-zA-Z0-9]");
          RESubExpression* pSubExpression = new RESubExpression(0, INT_MAX);
          pSubExpression->_pcChild = pNew;
          pNew = pSubExpression;
        }
        break;

      case 's':
        { // Whitespace
          pNew = new RECharClass(" \t\r\n", true);
        }
        break;

      case 'S':
        { // Non-whitespace
          pNew = new RECharClass(" \t\r\n", false);
        }
        break;

      default:
        pNew = new REChar(*pzPattern);
        break;
      }
      break;

    default:
      pNew = new REChar(*pzPattern);
      break;
    }
    patternStack.push(pNew);
    pzPattern++;
  }
  if (!substringStack.empty())
  {
    goto error;
  }


  // wrap in a block to avoid compiler warnings during goto and bypassing variable-decls
  {
    REBase
      *pFirst = 0;
    while (!patternStack.empty())
    {
      REBase* pFront = patternStack.top();
      patternStack.pop();
      pFront->_pcNext = pFirst;
      pFirst = pFront;
    }
    return pFirst;
  }

error:
  while(!patternStack.empty())
  {
    delete patternStack.top();
    patternStack.pop();
  }
  delete pNew;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Match

bool
RegExp::Match(int* pnCharactersMatched, REBase* pREBase, const char* pzString, bool isIgnoreCase, SubStringList* pcSubStringList)
{
  REContext context(isIgnoreCase);
  context._pcSubStringList = pcSubStringList;
  if (pREBase->Match(&context, pzString))
  {
    if (pnCharactersMatched)
    {
      *pnCharactersMatched = context._nCharactersMatched;
    }
    return true;
  }
  if (pnCharactersMatched)
  {
    *pnCharactersMatched = 0;
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////
// Match

bool
RegExp::Match(int* pnCharactersMatched, const char* pzPattern, const char* pzString, bool isIgnoreCase, SubStringList* pcSubStringList)
{
  REBase* pRegExp = CompileRegExp(pzPattern);
  if (pRegExp)
  {
    bool isMatch = Match(pnCharactersMatched, pRegExp, pzString, isIgnoreCase, pcSubStringList);
    delete pRegExp;
    return isMatch;
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////
// MatchWildcard

bool            
RegExp::MatchWildcard(int* pnCharactersMatched, const char* pzPattern, const char* pzString, bool isIgnoreCase)
{
  REBase* pRegExp = CompileWildcardExp(pzPattern);
  if (pRegExp)
  {
    bool isMatch = Match(pnCharactersMatched, pRegExp, pzString, isIgnoreCase, NULL);
    delete pRegExp;
    return isMatch;
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////
// Search

bool
RegExp::Search(int* pnCharactersMatched, const char** ppzMatchFound, REBase* pREBase, const char* pzString, bool isIgnoreCase, SubStringList* pcSubStringList)
{
  if (pzString)
  {
    while (*pzString)
    {
      if (Match(pnCharactersMatched, pREBase, pzString, isIgnoreCase, pcSubStringList))
      {
        if (ppzMatchFound)
        {
          *ppzMatchFound = pzString;
        }
        return true;
      }
      pzString++;
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////
// Search

bool
RegExp::Search(int* pnCharactersMatched, const char** ppzMatchFound, const char* pzPattern, const char* pzString, bool isIgnoreCase, SubStringList* pcSubStringList)
{
  REBase* pREBase = CompileRegExp(pzPattern);
  if (pREBase)
  {
    bool isFound = Search(pnCharactersMatched, ppzMatchFound, pREBase, pzString, isIgnoreCase, pcSubStringList);
    delete pREBase;
    return isFound;
  }
  return false;
}

  } // namespace Util 
} // namespace Hue
