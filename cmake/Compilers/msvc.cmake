# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

# MSVC >= 19.29
set(MSVC_SUPPORTS_VERSION 19.29.30140.0)

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS MSVC_SUPPORTS_VERSION)
    message(FATAL_ERROR "AscEmu requires version ${MSVC_SUPPORTS_VERSION} to build but found ${CMAKE_CXX_COMPILER_VERSION}")
else ()
    message(STATUS "Minimum version MSVC required is ${MSVC_SUPPORTS_VERSION}, found ${CMAKE_CXX_COMPILER_VERSION} - success")
endif ()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER_ID}")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# windows math include does not define constants by default.
# set this definition so it does.
# also set NOMINMAX so the min and max functions are not overwritten with macros.
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-D_USE_MATH_DEFINES)
add_definitions(-DNOMINMAX)

# set defines for MSVC
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /std:c++20 /EHa /MP /bigobj")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /EHa /MP /bigobj")

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

# install libraries for windows build (libmysql.dll)
install(FILES ${INSTALLED_DEPENDENCIES} DESTINATION .)
