# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

function(bdm_add_executable TARGET)
  cmake_parse_arguments(ARG "" "" "SOURCES;HEADERS;LIBRARIES" ${ARGN})
  add_executable(${TARGET} ${ARG_SOURCES} ${ARG_HEADERS})
  target_link_libraries(${TARGET} ${ARG_LIBRARIES})
endfunction()

function(build_shared_library TARGET)
  cmake_parse_arguments(ARG "" "" "SOURCES;HEADERS;LIBRARIES" ${ARGN})
  add_library(${TARGET} SHARED ${ARG_SOURCES} ${ARG_HEADERS})
  target_link_libraries(${TARGET} ${ARG_LIBRARIES})
endfunction()

function(fix_macos_opencl_header_issue)
  if(APPLE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/opencl/cl2.hpp")
      execute_process(COMMAND cp -a ${CMAKE_SOURCE_DIR}/third_party/opencl ${CMAKE_BINARY_DIR})
    elseif(EXISTS "$ENV{BDMSYS}/third_party/opencl/cl2.hpp")
      execute_process(COMMAND cp -a $ENV{BDMSYS}/third_party/opencl ${CMAKE_BINARY_DIR})
    elseif(EXISTS "$ENV{BDMSYS}/opencl/cl2.hpp")
      execute_process(COMMAND cp -a $ENV{BDMSYS}/opencl ${CMAKE_BINARY_DIR})
    else()
      message(FATAL_ERROR "The BioDynaMo environment is not set up correctly. Please execute 'source <path-to-bdm-installation>/bin/thisbdm.sh' and retry this command.")
    endif()
    include_directories("${CMAKE_BINARY_DIR}/opencl")
  endif()
endfunction()
