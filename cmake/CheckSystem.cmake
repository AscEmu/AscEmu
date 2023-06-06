# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# correctly switches from -std=gnu++2a to -std=c++2a.
set(CXX_EXTENSIONS OFF)

# set runtime binary where all compiled (before install) binary will compiled in
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set build type on unix if it wasn't defined by user
if (UNIX)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
    endif()

    if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -lc++experimental")
    elseif (APPLE)
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -lc++")
    endif ()
endif ()

# set RPATH-handing (CMake parameters)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# we have our own custom modules and dep modules that we use. This tells cmakes where to find them.
list(APPEND CMAKE_MODULE_PATH 
    ${CMAKE_SOURCE_DIR}/cmake/Modules)

# get git information
include(cmake/Modules/AEGitRevision.cmake)

# apply options settings
include(cmake/Modules/AEConfigureFiles.cmake)

# get architecture type and set architecture identifier
include(cmake/Modules/AEConfigureArch.cmake)

# default definitions
# -DPREFIX=\"${ASCEMU_SCRIPTLIB_PATH}\"
add_definitions(-DHAVE_CONFIG_H)

mark_as_advanced(
    ZLIB_LIBRARIES
    ZLIB_INCLUDE_DIRS
    PCRE_LIBRARIES
    PCRE_INCLUDE_DIR
    OPENSSL_LIBRARIES
    OPENSSL_INCLUDE_DIR
    MYSQL_LIBRARY
    MYSQL_INCLUDE_DIR
    BZIP2_LIBRARIES
    BZIP2_INCLUDE_DIRS
)
