# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# Clang >= 17.0.0
set(CLANG_SUPPORTS_VERSION 17.0.0)

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS CLANG_SUPPORTS_VERSION)
    message(FATAL_ERROR "AscEmu requires version ${CLANG_SUPPORTS_VERSION} to build but found ${CMAKE_CXX_COMPILER_VERSION}")
else ()
    message(STATUS "Minimum version Clang required is ${CLANG_SUPPORTS_VERSION}, found ${CMAKE_CXX_COMPILER_VERSION} - success")
endif ()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER}")

# check support for unordered_map/set
add_compile_options(-DHAS_CXX0X)

if (IS_64BIT)
    add_compile_options(-fPIC)
endif ()

if (BUILD_WITH_WARNINGS)
    add_compile_options(-Wall -Wextra)
else ()
    add_compile_options(-w)
endif ()

# ==== Fast linker & debug info optimization ====
# Prefer Mold, then LLD, fallback to gold; add Split DWARF for faster debug builds.
# Guard to avoid double injection if included multiple times.
if(NOT DEFINED FAST_LINKER_CONFIGURED)
  set(FAST_LINKER_CONFIGURED ON)
  set(SELECTED_LINKER "default")

  # Try Mold first
  execute_process(
    COMMAND ${CMAKE_C_COMPILER} -fuse-ld=mold -Wl,--version
    OUTPUT_VARIABLE LD_VERSION
    ERROR_QUIET
  )
  if("${LD_VERSION}" MATCHES "mold")
    add_link_options("-fuse-ld=mold")
    set(SELECTED_LINKER "Mold")
    message(STATUS "Linker: Using Mold")
  else()
    # Try next LLD
    execute_process(
      COMMAND ${CMAKE_C_COMPILER} -fuse-ld=lld -Wl,--version
      OUTPUT_VARIABLE LD_VERSION
      ERROR_QUIET
    )
    if("${LD_VERSION}" MATCHES "LLD")
      add_link_options("-fuse-ld=lld")
      set(SELECTED_LINKER "LLD")
      message(STATUS "Linker: Using LLD")
    else()
      # Fallback to gold
      execute_process(
        COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version
        OUTPUT_VARIABLE LD_VERSION
        ERROR_QUIET
      )
      if("${LD_VERSION}" MATCHES "GNU gold")
        add_link_options("-fuse-ld=gold")
        set(SELECTED_LINKER "gold")
        message(STATUS "Linker: Using GNU gold")
      else()
        message(STATUS "Linker: Using system default")
      endif()
    endif()
  endif()

  # Minor I/O improvement on compile
  add_compile_options(-pipe)
endif()
# ==== End fast linker block ====

# === Debug info & faster relinks (single-config friendly) ===
# Avoid generator-expressions for Debug+RelWithDebInfo to prevent misparsing.
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  add_compile_options(-gsplit-dwarf)

  if(SELECTED_LINKER STREQUAL "Mold" OR SELECTED_LINKER STREQUAL "gold" OR SELECTED_LINKER STREQUAL "LLD")
    add_link_options(LINKER:--gdb-index)
  endif()
  # System default linker (usually GNU bfd ld) does not support gdb-index as efficiently as other linkers
  # Keep disabled for compatibility issues
endif()
# === End debug info block ===
