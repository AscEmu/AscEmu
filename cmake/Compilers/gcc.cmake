# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# GCC >= 9.0.0
set(GCC_SUPPORTS_VERSION 9.0.0)

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS GCC_SUPPORTS_VERSION)
    message(FATAL_ERROR "AscEmu requires version ${GCC_SUPPORTS_VERSION} to build but found ${CMAKE_CXX_COMPILER_VERSION}")
else ()
    message(STATUS "Minimum version GCC required is ${GCC_SUPPORTS_VERSION}, found ${CMAKE_CXX_COMPILER_VERSION} - success")
endif ()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER}")

# check support for unordered_map/set
add_definitions(-DHAS_CXX0X)

# apply base flags
add_compile_options(-O2 -std=c++2a)

if (IS_64BIT)
    add_compile_options(-fPIC)
endif ()

if (BUILD_WITH_WARNINGS)
    add_compile_options(-Wall -Wextra)
else ()
    add_compile_options(-w)
endif ()
