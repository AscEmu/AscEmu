# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
message(STATUS "Applying settings for Windows system")

set(LIBS_DIR ${CMAKE_INSTALL_PREFIX}/bin)
# set default install prefix if it wasn't setted up
add_definitions(-DWIN32)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "C:/AscEmu" CACHE PATH "Install path prefix" FORCE)
endif()

# set source paths for libraries
if(IS_64BIT)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(MYSQL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/libmysql_debug_x64.lib CACHE INTERNAL "MYSQL libraries." FORCE)
        set(OPENSSL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/libeay32_debug_x64.lib CACHE INTERNAL "OpenSSL libraries." FORCE)
    else()
        set(MYSQL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/libmysql_release_x64.lib CACHE INTERNAL "MYSQL libraries." FORCE)
        set(OPENSSL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/libeay32_release_x64.lib CACHE INTERNAL "OpenSSL libraries." FORCE)
    endif()
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(MYSQL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/libmysql_debug_win32.lib CACHE INTERNAL "MYSQL libraries." FORCE)
        set(OPENSSL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/libeay32_debug_win32.lib CACHE INTERNAL "OpenSSL libraries." FORCE)
    else()
        set(MYSQL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/libmysql_release_win32.lib CACHE INTERNAL "MYSQL libraries." FORCE)
        set(OPENSSL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/libeay32_release_win32.lib CACHE INTERNAL "OpenSSL libraries." FORCE)
    endif()
endif()

# set dependencies include dirs paths
set(OPENSSL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dep/openssl CACHE INTERNAL "OpenSSL include dir." FORCE)
set(MYSQL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dep/mysql CACHE INTERNAL "MYSQL include dir." FORCE)

#Needed for socket stuff and crash handler
set(EXTRA_LIBS 
    ${EXTRA_LIBS}
    ws2_32.lib
    dbghelp.lib
)

#check for including our dependencies
if(IS_64BIT)
   set(DEPENDENCY_DLLS ${CMAKE_SOURCE_DIR}/dep/dll64)
else()
   set(DEPENDENCY_DLLS ${CMAKE_SOURCE_DIR}/dep/dll)
endif()

#Install libmysql.dll required for our core to run.
set(INSTALLED_DEPENDENCIES
    ${DEPENDENCY_DLLS}/libmysql.dll
    ${DEPENDENCY_DLLS}/libeay32.dll
)

#check for db update files
set(PATH_DB_LOGON_UPDATES ${CMAKE_SOURCE_DIR}/sql/logon/updates/)
set(PATH_DB_CHARACTER_UPDATES ${CMAKE_SOURCE_DIR}/sql/character/updates/)
set(PATH_DB_WORLD_UPDATES ${CMAKE_SOURCE_DIR}/sql/world/updates/)

set(INSTALL_LOGON_UPDATES ${PATH_DB_LOGON_UPDATES})
set(INSTALL_CHARACTER_UPDATES ${PATH_DB_CHARACTER_UPDATES})
set(INSTALL_WORLD_UPDATES ${PATH_DB_WORLD_UPDATES})

if(MSVC)
    include(${CMAKE_SOURCE_DIR}/cmake/Compilers/msvc.cmake)
else()
    message(FATAL_ERROR "Compiler is not supported")
endif()
