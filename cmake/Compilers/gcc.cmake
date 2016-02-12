# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
    message(FATAL_ERROR "Unsupported gcc version")
endif()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER_ID}")
add_definitions(-DHAS_CXX0X)

# apply base flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2")

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
