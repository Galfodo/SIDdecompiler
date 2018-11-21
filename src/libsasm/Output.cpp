
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include "HueUtilString.h"
#include "Output.h"

namespace SASM {

static outputstring_pf
  s_StdoutCallback,
  s_StderrCallback;

outputstring_pf sasm_setstdout_callback(outputstring_pf pfOutputStdout) {
  auto old = s_StdoutCallback;
  s_StdoutCallback = pfOutputStdout;
  return old;
}

outputstring_pf sasm_setstderr_callback(outputstring_pf pfOutputStderr) {
  auto old = s_StderrCallback;
  s_StderrCallback = pfOutputStderr;
  return old;
}

int sasm_vfprintf(FILE* file, const char* format, va_list arglist) {
  assert(file);
  Hue::Util::String output;
  output.vsprintf(format, arglist);
  if (file == stdout && s_StdoutCallback || file == stderr && s_StderrCallback) {
    if (file == stdout) {
      s_StdoutCallback(output.c_str());
    } else if (file == stderr) {
      s_StderrCallback(output.c_str());
    }
  }
  return fprintf(file, "%s", output.c_str());
}

int sasm_fprintf(FILE* file, const char* format, ...) {
  int ret;
  va_list arglist;
  assert(file);
  va_start(arglist, format);
  ret = sasm_vfprintf(file, format, arglist);
  va_end(arglist);
  return ret;
}

int sasm_printf(const char* format, ...) {
  int ret;
  va_list arglist;
  va_start(arglist, format);
  ret = sasm_vfprintf(stdout, format, arglist);
  va_end(arglist);
  return ret;
}

}
