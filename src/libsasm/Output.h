#ifndef SASM_OUTPUT_H_INCLUDED
#define SASM_OUTPUT_H_INCLUDED

#include <stdio.h>

namespace SASM {

typedef void (*outputstring_pf)(const char* str);

int             sasm_vfprintf(FILE* file, const char* format, va_list arglist);
int             sasm_fprintf(FILE* file, const char* format, ...);
int             sasm_printf(const char* format, ...);
outputstring_pf sasm_setstdout_callback(outputstring_pf pfOutputStdout);
outputstring_pf sasm_setstderr_callback(outputstring_pf pfOutputStderr);

}

#endif
