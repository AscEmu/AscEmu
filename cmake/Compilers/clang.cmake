# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# Clang >= 8.0.0
set(CLANG_SUPPORTS_VERSION 8.0.0)

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS CLANG_SUPPORTS_VERSION)
    message(FATAL_ERROR "AscEmu requires version ${CLANG_SUPPORTS_VERSION} to build but found ${CMAKE_CXX_COMPILER_VERSION}")
else ()
    message(STATUS "Minimum version Clang required is ${CLANG_SUPPORTS_VERSION}, found ${CMAKE_CXX_COMPILER_VERSION} - success")
endif ()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER}")

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
