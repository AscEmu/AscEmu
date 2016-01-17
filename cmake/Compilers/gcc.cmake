# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS REQUIRED_MIN_GCC_VERSION)
    message(FATAL_ERROR "Unsupported gcc version")
endif()

message(STATUS "Applying settings for \"GNU GCC\" compiler")

add_definitions(-DHAS_CXX0X)

#apply platform specific flags
if(APPLE)
    set( EXTRA_LIBS "${EXTRA_LIBS} -framework Carbon" )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set( EXTRA_LIBS ${EXTRA_LIBS} dl)
endif()

# apply base flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wno-deprecated")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated")

# apply flags for debug build
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
