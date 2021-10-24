# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

message(STATUS "Applying settings for Windows system")

set(LIBS_DIR ${CMAKE_INSTALL_PREFIX}/bin)

# set default install prefix if it wasn't setted up
add_definitions(-DWIN32)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "C:/AscEmu" CACHE PATH "Install path prefix" FORCE)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/Modules/FindMySQL.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Modules/FindOpenSSL.cmake)

# needed for socket stuff and crash handler
set(EXTRA_LIBS 
    ${EXTRA_LIBS}
    ws2_32.lib
    dbghelp.lib
)

# install libmysql.dll required for our core to run.
set(INSTALLED_DEPENDENCIES
    ${MYSQL_DLL}
)

# check for db update files
set(PATH_DB_FILES ${CMAKE_SOURCE_DIR}/sql/)

set(INSTALL_DB_FILES ${PATH_DB_FILES})

if(MSVC)
    include(${CMAKE_SOURCE_DIR}/cmake/Compilers/msvc.cmake)
else()
    message(FATAL_ERROR "Compiler is not supported")
endif()
