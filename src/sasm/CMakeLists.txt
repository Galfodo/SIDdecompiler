#
# Copyright (c) 2023 Erwin Rol <erwin@erwinrol.com>
#
# SPDX-License-Identifier: MIT
#

add_executable(
  sasm 

  sasm.cpp
)

target_link_libraries(
  sasm
PUBLIC
  HueUtil
  libsasm
  libsasmdisasm
)

install(TARGETS sasm RUNTIME DESTINATION bin)

