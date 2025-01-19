# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# MSVC >= 19.29
set(MSVC_SUPPORTS_VERSION 19.29.30140.0)

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS MSVC_SUPPORTS_VERSION)
    message(FATAL_ERROR "AscEmu requires version ${MSVC_SUPPORTS_VERSION} to build but found ${CMAKE_CXX_COMPILER_VERSION}")
else ()
    message(STATUS "Minimum version MSVC required is ${MSVC_SUPPORTS_VERSION}, found ${CMAKE_CXX_COMPILER_VERSION} - success")
endif ()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER_ID}")

add_compile_options(-D_CRT_SECURE_NO_WARNINGS)

# windows math include does not define constants by default.
# set this definition so it does.
# also set NOMINMAX so the min and max functions are not overwritten with macros.
add_compile_options(-DWIN32_LEAN_AND_MEAN)
add_compile_options(-D_USE_MATH_DEFINES)
add_compile_options(-DNOMINMAX)

# set defines for MSVC
add_compile_options(/std:c++20 /EHa /MP /bigobj)

# set build platform specific settings (x86/x64)
if (NOT IS_64BIT)
    add_link_options(/LARGEADDRESSAWARE)
endif ()

if (TREAT_WARNINGS_AS_ERRORS)
    add_compile_options(/WX)
endif ()

# enable/disable warnings
# dll warning 4251 disabled by default.
if (BUILD_WITH_WARNINGS)
    add_compile_options(/W3 /wd4251 /wd4820 /wd4062 /wd4061 /wd5045)
else ()
    add_compile_options(/W0)
endif ()
