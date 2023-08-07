#
# Copyright (c) 2023 Erwin Rol <erwin@erwinrol.com>
#
# SPDX-License-Identifier: MIT
#

add_executable(
  SIDdisasm

  siddisasm.cpp
  STHubbardRipper.cpp
  STMemBuf.cpp
  st_sidfile.cpp
  STSIDRipper.cpp
)

target_link_libraries(
  SIDdisasm
PUBLIC
  HueUtil
  libsasm
  libsasmemu
)

install(TARGETS SIDdisasm RUNTIME DESTINATION bin)
