#
# Copyright (c) 2023 Erwin Rol <erwin@erwinrol.com>
#
# SPDX-License-Identifier: MIT
#

include_directories(
  ./
  ./HueUtil/
  ./libsasm/
  ./libsasmdisasm/
  ./libsasmemu/
 ./libSID/
)

add_subdirectory(HueUtil)
add_subdirectory(libsasm)
add_subdirectory(libsasmdisasm)
add_subdirectory(libsasmemu)
add_subdirectory(sasm)
add_subdirectory(sasmSIDdump)
add_subdirectory(SIDcompare)
add_subdirectory(SIDdisasm)
