
#include <stdio.h>

#include "Parser.h"
#include "Assembler.h"
#include "Output.h"

#define UNITTEST 0

#if UNITTEST
void TokenizerTest();
#endif

using namespace SASM;

static void OutputString(const char* str) {
  int debug = 0;
}

const char* nextArg(int& currentIndex, int argc, char** argv, bool allowNone) {
  ++currentIndex;
  if (currentIndex < argc) {
    if (argv[currentIndex][0] != '-') {
      return argv[currentIndex];
    }
  }
  if (allowNone) {
    return "";
  } else {
    return NULL;
  }
}

void usage() {
  printf(
    "%s\n"
    "Usage:\n"
    "SASM [-o outfile] <infile>\n",
    Assembler::version()
  );
}

int main(int argc, char** argv) {
  Hue::Util::String sFilename;
  Hue::Util::String sOutname;
  sasm_setstdout_callback(OutputString);
  sasm_setstderr_callback(OutputString);
  int max_errors = 10;
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case '-':
        if (strcmp(argv[i], "--version") == 0) goto version_;
        if (strcmp(argv[i], "--help") == 0) goto help_;
        goto unknown_;
      case 'v':
      version_:
        printf("%s\n", Assembler::version());
        exit(0);
      case 'h':
      help_:
        usage();
        exit(0);
        break;
      case 'o':
        sOutname = nextArg(i, argc, argv, true);
        break;
      default:
      unknown_:
        fprintf(stderr, "Unknown option '%s'\n", argv[i]);
        exit(-1);
      }
    } else {
      sFilename = argv[i];
    }
  }
  printf("%s\n", Assembler::version());
  if (sFilename.empty()) {
    printf("No input.\n");
    return 0;
  }
  if (sOutname.empty()) {
    sOutname = sFilename;
    sOutname.append(".out");
  }
#if UNITTEST
  TokenizerTest();
  return 0;
#endif

  //Parser
  //  parser;

  ////parser.addFile("../../test/unified_player2.s");   
  //parser.addFile("../../test/monty.asm");   

  Assembler
    assembler;
#if 0
  auto data = assembler.assemble_string(
    "  .org $1000                                                                                 \n"
    "                                                                                             \n"
    "  lda #6                                                                                     \n"
    "  sta $d020                                                                                  \n"
    "-                                                                                            \n"
    "  inc $d021                                                                                  \n"
    "  bne -                                                                                      \n"
    "  rts                                                                                        \n"
    "                                                                                             \n"
    "                                                                                             \n"
    "                                                                                             \n");
#else
  //auto data = assembler.assemble_file("../test/monty.asm");
  auto data = assembler.assemble_file(sFilename.c_str());
#endif
  assembler.fprintErrors(stderr, max_errors);
  if (assembler.errorcount() == 0) {
    assembler.write(sOutname.c_str(), data);
  } 
  return 0;
}