# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# Clang >= 11.0.0
set(CLANG_SUPPORTS_VERSION 11.0.0)

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS CLANG_SUPPORTS_VERSION)
    message(FATAL_ERROR "AscEmu requires version ${CLANG_SUPPORTS_VERSION} to build but found ${CMAKE_CXX_COMPILER_VERSION}")
else ()
    message(STATUS "Minimum version Clang required is ${CLANG_SUPPORTS_VERSION}, found ${CMAKE_CXX_COMPILER_VERSION} - success")
endif ()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER}")

add_definitions(-DHAS_CXX0X)

# apply base flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -std=c17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++20")

if (IS_64BIT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif ()

if (BUILD_WITH_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif ()
