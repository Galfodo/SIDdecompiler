#
# Copyright (c) 2023 Erwin Rol <erwin@erwinrol.com>
#
# SPDX-License-Identifier: MIT
#

add_library(
  libsasmemu

  C64MachineState.cpp
  DebugEvaluators.cpp
  DebuggerState.cpp
  Monitor.cpp
  Profiling.cpp
  TraceOperand.cpp
)

target_link_libraries(
  libsasmemu
PUBLIC
  libsasm
  libsasmdisasm
  HueUtil
)
