#ifndef HUE_UTIL_PROGRAMOPTION_H_INCLUDED
#define HUE_UTIL_PROGRAMOPTION_H_INCLUDED

// Version 6 17.11.2017

#include <vector>
#include <stdlib.h>
#include <assert.h>

#define HUE_USE_FAST_STRING_SEARCH 0
#include "HueUtilString.h"

namespace Hue
{
  namespace Util
  {
    typedef Hue::Util::String OptionString;

    // This is the base class for program options
    class ProgramOption
    {
    public:
      // Option name, e.g "define"
      virtual const char* GetName() const { return name.c_str(); }                

      // Option arguments, e.g "<name>=value"
      virtual const char* GetArg() const { return arg.c_str(); }                  

      // Option description, e.g "Define preprocessor macro"
      virtual const char* GetDescription() const { return description.c_str(); }

      // This method parses option argumens. "arg" points to the first character after the option name
      virtual bool ParseOption(const char*& arg, int& iArg, int const argc, char const* const* argv) = 0;

      // Max instances of an option. 0 means unlimited.
      virtual int GetMaxInstances() const { return maxInstances; }

      // Initialize to default value
      virtual void Initialize() = 0;

      // Convert to string
      virtual OptionString ToString(bool defaultValue) = 0;

      // ProgramOptionList uses this property to keep count of option instances.
      int Occurences;
    protected:
      OptionString name;
      OptionString arg;
      OptionString description;
      int maxInstances;
      ProgramOption(const char* Name, const char* Arg, const char* Description, int MaxInstances)
      {
        name = Name;
        arg = Arg;
        description = Description;
        maxInstances = MaxInstances;
        Occurences = 0;
      }

      const char* nextArg(int& iArg, int const argc, char const* const* argv) {
        if (iArg + 1 < argc) {
          return argv[++iArg];
        } else {
          return NULL;
        }
      }

    };

    // This option class records a single string
    class StringOption : public ProgramOption
    {
    public:
      OptionString Value;
      OptionString Default;

      StringOption(const char* Name, const char* Arg, const char* Description, const char* default_value, int MaxInstances = 1) :
        ProgramOption(Name, Arg, Description, MaxInstances), Value(default_value), Default(default_value)
      {
      }

      virtual void Initialize() {
        Value = Default;
      }

      virtual OptionString ToString(bool defaultValue) {
        if (defaultValue && Default.empty()) {
          return "";
        }
        auto result = OptionString::static_printf("%s=%s", name.c_str(), defaultValue ? Default.c_str() : Value.c_str());
        return result;
      }

      virtual bool
      ParseOption(const char*& param, int& iArg, int const argc, char const* const* argv)
      {
        if (param)
        {
          if (*param == '\0') {
            param = "";
            const char* tmp = nextArg(iArg, argc, argv);
            return ParseOption(tmp, iArg, argc, argv);
          }
          if (*param == '=')
            ++param;

          Value = param;
          param = "";
        }
        return true;
      }
    };

    // This option class records a series of strings, e.g multiple "-Dmacro=value"
    // It can also be used for setting parameters, e.g -path=../bin
    class StringListOption : public ProgramOption
    {
    public:
      OptionString::List ValueList;

      StringListOption(const char* Name, const char* Arg, const char* Description, int MaxInstances = 0) :
        ProgramOption(Name, Arg, Description, MaxInstances)
      {
      }

      virtual void Initialize() {
        ValueList.clear();
      }

      virtual OptionString ToString(bool defaultValue) {
        auto result = OptionString::static_printf("%s=%s", name.c_str(), defaultValue ? "" : ValueList.join(";").c_str());
        return result;
      }

      virtual bool
      ParseOption(const char*& param, int& iArg, int const argc, char const* const* argv)
      {
        if (param)
        {
          if (*param == '\0') {
            param = "";
            const char* tmp = nextArg(iArg, argc, argv);
            return ParseOption(tmp, iArg, argc, argv);
          } else {
            if (*param == '=')
              ++param;
            if (*param) {
              OptionString value = param;
              ValueList.push_back(value);
              param = "";
            }
            return true;
          }
        }
        return false;
      }
    };

    // This option records a boolean value. When the option occurs, the current value is negated. Always
    // defaults to false
    class FlagOption : public ProgramOption
    {
    public:
      bool Value;

      FlagOption(const char* Name, const char* Arg, const char* Description) :
        ProgramOption(Name, Arg, Description, 1)
      {
        Value   = false;
      }

      virtual void Initialize() {
        Value = false;
      }

      virtual OptionString ToString(bool defaultValue) {
        if (defaultValue) 
        {
          return "";
        }
        else
        {
          char temp[256];
          sprintf(temp, "%s=%d", name.c_str(), Value ? 1 : 0);
          return OptionString(temp);
        }
      }

      virtual bool ParseOption(const char*& param, int& iArg, int const argc, char const* const* argv)
      {
        Value = !Value;
        return true;
      }

    };

    // This option class records an integer value in a specified range.
    class IntegerOption : public ProgramOption
    {
    public:
      int Default;
      int Value;
      int Min;
      int Max;
      int Hex;

      IntegerOption(const char* Name, const char* Arg, const char* Description, int DefaultValue, int MinValue, int MaxValue, int hex_digits = 0) :
        ProgramOption(Name, Arg, Description, 1)
      {
        Default       = DefaultValue;
        Value         = DefaultValue;
        Min           = MinValue;
        Max           = MaxValue;
        Hex           = hex_digits;
      }

      virtual void Initialize() {
        Value = Default;
      }

      virtual OptionString ToString(bool defaultValue) {
        char temp[256];
        int val = defaultValue ? Default : Value;
        if (Hex) {
          if (Hex <= 2)
            sprintf(temp, "%s=0x%02x", name.c_str(), val);
          else if (Hex <= 4)
            sprintf(temp, "%s=0x%04x", name.c_str(), val);
          else
            sprintf(temp, "%s=0x%08x", name.c_str(), val);
        } else {
          sprintf(temp, "%s=%d", name.c_str(), val);
        }
        return OptionString(temp);
      }

      virtual bool
      ParseOption(const char*& param, int& iArg, int const argc, char const* const* argv)
      {
        if (param) {
          if (*param) {
            int value = 0;
            if (*param == '=') 
              ++param;
            if (*param == '$' || (*param == '0' && *(param + 1) == 'x')) {
              param += (*param == '$') ? 1 : 2;
              sscanf(param, "%x", &value);
            }
            else 
              value = atoi(param);
            if (value >= Min && value <= Max) {
              Value = value;
              param = "";
              return true;
            }
          } else {
            param = "";
            const char* tmp = nextArg(iArg, argc, argv);
            return ParseOption(tmp, iArg, argc, argv);
          }
        }
        return false;
      }
    };

    class AliasOption : public ProgramOption
    {
    public:
      ProgramOption* Alias;

      AliasOption(const char* Name, const char* Description, ProgramOption* alias) :
        ProgramOption(Name, "", Description, 1)
      {
        assert(alias);
        Alias = alias;
      }

      virtual void Initialize() {
        Alias->Initialize();
      }

      virtual OptionString ToString(bool defaultValue) {
        return Alias->ToString(defaultValue);
      }

      virtual bool ParseOption(const char*& param, int& iArg, int const argc, char const* const* argv)
      {
        return Alias->ParseOption(param, iArg, argc, argv);
      }

    };

    // This class keeps track of all available options
    class ProgramOptionList : public std::vector<ProgramOption*>
    {
    public:
      // Initialize all options to their default value
      virtual void InitializeAll()
      {
        for (iterator it = begin(); it != end(); ++it)
        {
          (*it)->Initialize();
        }
      }

      // Print option documentation to stdout
      virtual void
      PrintHelp()
      {
        size_t maxOptionLen = 0;
        size_t maxArgLen = 0;
        for (iterator it = begin(); it != end(); ++it)
        {
          size_t optionLen = strlen((*it)->GetName());
          size_t argLen = strlen((*it)->GetArg());
          maxOptionLen = optionLen > maxOptionLen ? optionLen : maxOptionLen;
          maxArgLen = argLen > maxArgLen ? argLen : maxArgLen;
        }
        char format[256];
        sprintf(format, "-%%-%ds: %%s%%s\n", (int)(maxOptionLen + maxArgLen));
        for (iterator it = begin(); it != end(); ++it)
        {
          OptionString defaultString = (*it)->ToString(true);
          defaultString.truncate_up_to_including_last("=");
          if (!defaultString.empty()) {
            defaultString.prepend(" (default: ").append(")");
          }
          OptionString opt = (*it)->GetName();
          opt.append((*it)->GetArg());
          printf(format, opt.c_str(), (*it)->GetDescription(), defaultString.c_str());
        }
      }

      // Checks the supplied option string against the list of available options, parses the string 
      // and checks + updates instance count for valid options. Prints an errormessage to stderr and 
      // returns false if anything went wrong.
      // Updates iArg for each option/argument consumed.
      virtual bool
      ParseOption(int& iArg, int const argc, char const* const* argv)
      {
        assert(iArg < argc);
        assert(argv);
        assert(argv[iArg]);
        const char* option = argv[iArg];
        const char* prevoption = NULL;
        if (*option == '-') {
          ++option;
          while (*option && prevoption != option) 
          {
            prevoption = option;
            for (iterator it = begin(); it != end(); ++it)
            {
              if (strncmp((*it)->GetName(), option, strlen((*it)->GetName())) == 0)
              {
                if ((*it)->GetMaxInstances() == 0 || (*it)->Occurences < (*it)->GetMaxInstances())
                {
                  ++(*it)->Occurences;
                  option += strlen((*it)->GetName());
                  if ((*it)->ParseOption(option, iArg, argc, argv)) {
                    break;
                  } else {
                    fprintf(stderr, "Option parse error: %s\n", (*it)->GetName());
                    return false;
                  }
                }
                else
                {
                  fprintf(stderr, "Too many occurences of option: %s\n", (*it)->GetName());
                }
              }
            }
            if (option == prevoption)
            {
              fprintf(stderr, "Unknown option: %s\n", option);
              return false;
            }
          }
        }
        return true;
      }

      bool ParseOption(char const* arg) {
        int i = 0;
        return ParseOption(i, 1, (char**)&arg);
      }

    };

  } // namespace Util

} // namespace Hue

#endif // !defined HUE_UTIL_PROGRAMOPTION_H_INCLUDED
