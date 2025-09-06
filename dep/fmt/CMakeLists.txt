# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

include(CheckSymbolExists)

set(strtod_l_headers stdlib.h)

if (APPLE)
    set(strtod_l_headers ${strtod_l_headers} xlocale.h)
endif ()

function(add_headers VAR)
    set(headers ${${VAR}})
    foreach (header ${ARGN})
        set(headers ${headers} include/fmt/${header})
    endforeach()
    set(${VAR} ${headers} PARENT_SCOPE)
endfunction()

# collect all public fmt headers.
add_headers(FMT_HEADERS
    args.h
    chrono.h
    color.h
    compile.h
    core.h
    format.h
    format-inl.h
    os.h
    ostream.h
    printf.h
    ranges.h
    std.h
    xchar.h)

# collect source files.
set(FMT_SOURCES
    src/format.cc
    src/os.cc)

# define the main fmt library (static).
add_library(fmt STATIC ${FMT_SOURCES} ${FMT_HEADERS})

# provide an alias target for modern CMake usage (fmt::fmt).
add_library(fmt::fmt ALIAS fmt)

# enable locale support if available
if (HAVE_STRTOD_L)
    target_compile_definitions(fmt
        PUBLIC
            FMT_LOCALE)
endif()

# define FMT_CONSTEVAL for C++20 usage.
target_compile_definitions(fmt
    PUBLIC
        FMT_CONSTEVAL=)

# add include directory for public headers.
target_include_directories(fmt
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
