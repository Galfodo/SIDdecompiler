#
# Copyright (c) 2023 Erwin Rol <erwin@erwinrol.com>
#
# SPDX-License-Identifier: MIT
#

add_executable(
  SIDcompare

  sidcompare.cpp
)

target_link_libraries(
  SIDcompare
PUBLIC
  HueUtil
  libsasm
  libsasmemu
)

install(TARGETS SIDcompare RUNTIME DESTINATION bin)
