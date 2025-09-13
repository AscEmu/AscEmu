# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# correctly switches from -std=gnu++2a to -std=c++2a.
set(CMAKE_CXX_EXTENSIONS OFF)

# set runtime binary where all compiled (before install) binary will compiled in
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/lib)

# we have our own custom modules and dep modules that we use. This tells cmakes where to find them.
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules ${CMAKE_MODULE_PATH})

# set build type on unix if it wasn't defined by user
if (UNIX)
    if (NOT CMAKE_BUILD_TYPE)
        message(STATUS "Build configuration was not detected, setting to \"Release\"")
        set(CMAKE_BUILD_TYPE "Release")
    else ()
        message(STATUS "Detected ${CMAKE_BUILD_TYPE} configuration")
    endif ()
endif ()

# set RPATH-handing (CMake parameters)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# get git information
include(${CMAKE_MODULE_PATH}/AEGitRevision.cmake)

# get group sources 
include(${CMAKE_MODULE_PATH}/AEGroupSources.cmake)

# apply options settings
include(${CMAKE_MODULE_PATH}/AEConfigureFiles.cmake)

# get architecture type and set architecture identifier
include(${CMAKE_MODULE_PATH}/AEConfigureArch.cmake)

# default definitions
# -DPREFIX=\"${ASCEMU_SCRIPTLIB_PATH}\"
add_compile_options(-DHAVE_CONFIG_H)

mark_as_advanced(
    ZLIB_LIBRARIES
    ZLIB_INCLUDE_DIRS
    OPENSSL_LIBRARIES
    OPENSSL_INCLUDE_DIR
    MYSQL_LIBRARY
    MYSQL_INCLUDE_DIR
    BZIP2_LIBRARIES
    BZIP2_INCLUDE_DIRS
)
