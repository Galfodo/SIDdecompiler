#
# Copyright (c) 2023 Erwin Rol <erwin@erwinrol.com>
#
# SPDX-License-Identifier: MIT
#

add_library(
  libsasm

  Assembler.cpp
  Assertion.cpp
  Label.cpp
  OpcodeDefs.cpp
  Output.cpp
  ParserContext.cpp
  Parser.cpp
  Section.cpp
  TextBuffer.cpp
  Token.cpp
  Tokenizer.cpp
  TokenizerTest.cpp
  TokenList.cpp
  TokenStream.cpp
  UnresolvedReference.cpp
  Util.cpp
)

target_link_libraries(
  libsasm
PUBLIC
  HueUtil
)

