# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

if(NOT COMPILER_SUPPORTS_CXX11)
    message(FATAL_ERROR "${CMAKE_CXX_COMPILER} does not support C++11 feature")
endif()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER}")
add_definitions(-DHAS_CXX0X)

# apply base flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++11")

if (IS_64BIT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

if (BUILD_WITH_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()
