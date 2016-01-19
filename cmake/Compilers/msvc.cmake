# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

if(MSVC_VERSION VERSION_LESS REQUIRED_MIN_MSVC_VERSION)
    message(FATAL_ERROR "This Visual studio version is not supported")
endif()

message(STATUS "Applying settings for \"Microsoft Visual Studio\" compiler")

add_definitions(-D_CRT_SECURE_NO_WARNINGS -DHAS_CXX0X)

#set defines for msvc
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /EHa /MP /bigobj")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa /MP /bigobj")

# apply flags for debug build
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /W3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3")

#This fixes PCH issues 'Inconsistent values for /Zm'
if(${CMAKE_CXX_FLAGS} MATCHES "(/Zm)([0-9]+)")
    string(REGEX REPLACE "(/Zm)([0-9]+)" "\\1${VISUALSTUDIO_COMPILERHEAPLIMIT}" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm${VISUALSTUDIO_COMPILERHEAPLIMIT}")
endif()

if(${CMAKE_C_FLAGS} MATCHES "(/Zm)([0-9]+)")
    string(REGEX REPLACE "(/Zm)([0-9]+)" "\\1${VISUALSTUDIO_COMPILERHEAPLIMIT}" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zm${VISUALSTUDIO_COMPILERHEAPLIMIT}")
endif()
