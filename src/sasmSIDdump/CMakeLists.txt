#
# Copyright (c) 2023 Erwin Rol <erwin@erwinrol.com>
#
# SPDX-License-Identifier: MIT
#

add_executable(
  sasmSIDdump

  cpu.c
  sasmSIDdump.cpp
)

target_link_libraries(
  sasmSIDdump
PUBLIC
  HueUtil
  libsasm
  libsasmemu
)

install(TARGETS sasmSIDdump RUNTIME DESTINATION bin)
