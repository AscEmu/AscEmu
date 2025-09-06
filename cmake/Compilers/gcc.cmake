# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# GCC >= 12.0.0
set(GCC_SUPPORTS_VERSION 12.0.0)
# TODO change to 13 when Debian 13 is released

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS GCC_SUPPORTS_VERSION)
    message(FATAL_ERROR "AscEmu requires version ${GCC_SUPPORTS_VERSION} to build but found ${CMAKE_CXX_COMPILER_VERSION}")
else ()
    message(STATUS "Minimum version GCC required is ${GCC_SUPPORTS_VERSION}, found ${CMAKE_CXX_COMPILER_VERSION} - success")
endif ()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER}")

# check support for unordered_map/set
add_compile_options(-DHAS_CXX0X)

# apply base flags (optimization level 2)
add_compile_options(-O2)

if (IS_64BIT)
    add_compile_options(-fPIC)
endif ()

if (BUILD_WITH_WARNINGS)
    add_compile_options(-Wall -Wextra)
else ()
    add_compile_options(-w)
endif ()

# ==== Fast linker & debug info optimization ====
# Prefer LLD, fallback to gold; add Split DWARF for faster debug builds.
# Guard to avoid double injection if included multiple times.
if(NOT DEFINED FAST_LINKER_CONFIGURED)
  set(FAST_LINKER_CONFIGURED ON)

  # Try LLD first
  execute_process(
    COMMAND ${CMAKE_C_COMPILER} -fuse-ld=lld -Wl --version
    OUTPUT_VARIABLE _ldver_lld
    ERROR_QUIET
  )
  if("${_ldver_lld}" MATCHES "LLD")
    add_link_options(-fuse-ld=lld)
    # Enable ICF (identical code folding) outside Debug to reduce size and sometimes link time
    add_link_options($<$<NOT:$<CONFIG:Debug>>:-Wl --icf=all>)
    message(STATUS "Linker: Using LLD")
  else()
    # Fallback to gold
    execute_process(
      COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl --version
      OUTPUT_VARIABLE _ldver_gold
      ERROR_QUIET
    )
    if("${_ldver_gold}" MATCHES "GNU gold")
      add_link_options(-fuse-ld=gold -Wl --threads)
      add_link_options($<$<NOT:$<CONFIG:Debug>>:-Wl --icf=safe>)
      message(STATUS "Linker: Using GNU gold")
    else()
      message(STATUS "Linker: Using system default")
    endif()
  endif()

  # Minor I/O improvement on compile
  add_compile_options(-pipe)
endif()
# ==== End fast linker block ====

# === Debug info & faster relinks (single-config friendly) ===
# Avoid generator-expressions for Debug+RelWithDebInfo to prevent misparsing.
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-gsplit-dwarf -fdebug-types-section)
    add_link_options(-Wl --gdb-index)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-gsplit-dwarf)
    add_link_options(-Wl --gdb-index)
  endif()
endif()
# === End debug info block ===
