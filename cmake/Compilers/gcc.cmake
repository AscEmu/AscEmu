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
