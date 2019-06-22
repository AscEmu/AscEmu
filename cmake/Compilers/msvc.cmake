# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

if(MSVC_VERSION VERSION_LESS 19.0.24210) #2015 3
    message(FATAL_ERROR "AscEmu requires at least Visual Studio 2015 update 3")
endif()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER_ID}")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#set defines for msvc
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /EHa /MP /bigobj")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa /MP /bigobj")

# set build platform specific settings (x86/x64)
if(NOT IS_64BIT)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
endif()

# enable/disable warnings
# dll warning 4251 disabled by default.
if (BUILD_WITH_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX /wd4251")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /wd4251")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
endif()
