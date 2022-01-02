# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

if (MSVC_VERSION VERSION_LESS 19.28)
    message(FATAL_ERROR "AscEmu requires at least Visual Studio 2019 update 16.9")
endif ()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER_ID}")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# set defines for msvc
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /std:c++17 /EHa /MP /bigobj")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /EHa /MP /bigobj")

# set build platform specific settings (x86/x64)
if (NOT IS_64BIT)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
endif ()

if (TREAT_WARNINGS_AS_ERRORS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
endif ()

# enable/disable warnings
# dll warning 4251 disabled by default.
if (BUILD_WITH_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Wall /wd4251 /wd4820 /wd4062 /wd4061 /wd5045")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall /wd4251 /wd4820 /wd4062 /wd4061 /wd5045")
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
endif ()
