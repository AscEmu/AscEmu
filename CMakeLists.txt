# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
project(Ascemu)

cmake_minimum_required(VERSION 3.16.3)

# set runtime binary where all compiled (before install) binary will compiled in
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set build type on unix if it wasn't defined by user
if (UNIX)
    if (NOT CMAKE_BUILD_TYPE)
        message(STATUS "Build configuration was not detected, setting to \"Release\"")
        set(CMAKE_BUILD_TYPE "Release")
    else ()
        message(STATUS "Detected ${CMAKE_BUILD_TYPE} configuration")
    endif ()

    if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -lc++experimental")
    elseif (APPLE)
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -lc++")
    else ()
        # link atomic library
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -latomic")
    endif ()
endif ()

# needed for windows
find_package(OpenSSL)

# cmake options
include(${CMAKE_SOURCE_DIR}/cmake/Options.cmake)

# perform system tasks
include(${CMAKE_SOURCE_DIR}/cmake/CheckSystem.cmake)

# add dependecies
add_subdirectory(dep)

# add executables
add_subdirectory(src)

# add script libraries
add_subdirectory(src/scripts)

# install db updates
install(DIRECTORY ${INSTALL_DB_FILES} DESTINATION sql FILES_MATCHING PATTERN "*.sql")
