# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

message(STATUS "Applying settings for Windows system")

# set default install prefix if it wasn't setted up
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "C:/AscEmu" CACHE PATH "Install path prefix" FORCE)
endif ()

find_package(MySQL)
find_package(OpenSSL)

# needed for socket stuff and crash handler
set(EXTRA_LIBS
    ws2_32.lib
    dbghelp.lib
)

if (MSVC)
    include(${CMAKE_SOURCE_DIR}/cmake/Compilers/msvc.cmake)
else ()
    message(FATAL_ERROR "Compiler is not supported")
endif ()

# check for db update files
set(PATH_DB_FILES ${CMAKE_SOURCE_DIR}/sql/)
set(INSTALL_DB_FILES ${PATH_DB_FILES})

# install libraries for windows build (libmysql.dll)
install(FILES ${MYSQL_DLL} DESTINATION .)
