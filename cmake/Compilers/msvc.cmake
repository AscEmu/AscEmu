# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# MSVC >= 19.29
set(MSVC_SUPPORTS_VERSION 19.29.30140.0)

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS MSVC_SUPPORTS_VERSION)
    message(FATAL_ERROR "AscEmu requires version ${MSVC_SUPPORTS_VERSION} to build but found ${CMAKE_CXX_COMPILER_VERSION}")
else ()
    message(STATUS "Minimum version MSVC required is ${MSVC_SUPPORTS_VERSION}, found ${CMAKE_CXX_COMPILER_VERSION} - success")
endif ()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER_ID}")

# windows math include does not define constants by default.
# set this definition so it does.
# also set NOMINMAX so the min and max functions are not overwritten with macros.
add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
    _USE_MATH_DEFINES
    NOMINMAX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set defines for MSVC
add_compile_options(
	/EHa 
	/MP 
	/bigobj 
	/permissive-
)

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
    add_compile_options(
	/W3
	/external:anglebrackets
	/external:W3
	/wd4251 # Suppress DLL-interface warning
	/wd4820 # Suppress padding warning
	/wd5045 # Suppress Spectre mitigation warning
)
else ()
    add_compile_options(/W0)
endif ()
