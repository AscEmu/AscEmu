# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

# set RPATH-handing (CMake parameters)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# we have our own custom modules and dep modules that we use. This tells cmakes where to find them.
list(APPEND CMAKE_MODULE_PATH 
    ${CMAKE_SOURCE_DIR}/cmake/Modules)

# get git information
include(${CMAKE_SOURCE_DIR}/cmake/Modules/AEGitRevision.cmake)

# apply options settings
include(${CMAKE_SOURCE_DIR}/cmake/Modules/AEConfigureFiles.cmake)

# get architecture type and set architecture identifier
include(${CMAKE_SOURCE_DIR}/cmake/Modules/AEConfigureArch.cmake)

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
