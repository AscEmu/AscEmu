# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)

if (NOT COMPILER_SUPPORTS_CXX17)
    message(FATAL_ERROR "AscEmu requires at least GCC 8! Current version ${CMAKE_CXX_COMPILER} does not support c++20 feature")
endif ()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER}")
add_definitions(-DHAS_CXX0X)

# apply base flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O2")

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
