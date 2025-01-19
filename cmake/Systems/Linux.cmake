# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

message(STATUS "Applying settings for Linux system")

set(EXTRA_LIBS ${EXTRA_LIBS} dl)

set(LIBS_DIR ${CMAKE_INSTALL_PREFIX}/lib)
add_compile_options(-DUSE_EPOLL)

# find required libraries
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(MySQL REQUIRED)
find_package(BZip2 REQUIRED)

if (CMAKE_COMPILER_IS_GNUCXX)
    include(${CMAKE_SOURCE_DIR}/cmake/Compilers/gcc.cmake)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    include(${CMAKE_SOURCE_DIR}/cmake/Compilers/clang.cmake)
else ()
    message(FATAL_ERROR "Compiler is not supported")
endif ()

# check for database update files
set(PATH_DB_FILES ${CMAKE_SOURCE_DIR}/sql/)
set(INSTALL_DB_FILES ${PATH_DB_FILES})
