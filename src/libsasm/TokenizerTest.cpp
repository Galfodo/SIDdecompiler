
#include <assert.h>
#include "Tokenizer.h"
#include "TokenStream.h"

using namespace SASM;

static const char*
  s_TestString = "  1234 56.78 0xdeadbeef $fce2 %11110000  ";

int64_t
  s_Integers[] = { 1234, 56, 0xdeadbeef, 0xfce2, 0xf0, -1 };

void TokenizerTest() {
  TokenStream
    stream(&Tokenizer::GetToken, s_TestString);

  int i = 0;

  Token
    token;

  while (stream.next(token)) {
    assert(s_Integers[i] >= 0);
    int64_t tokenval = token.integerValue();
    assert(tokenval == s_Integers[i]);
    ++i;
  }
}
