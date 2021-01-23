# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

message(STATUS "Applying settings for Windows system")

set(LIBS_DIR ${CMAKE_INSTALL_PREFIX}/bin)
# set default install prefix if it wasn't setted up
add_definitions(-DWIN32)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "C:/AscEmu" CACHE PATH "Install path prefix" FORCE)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/Modules/FindMySQL.cmake)

# set source paths for libraries
if(IS_64BIT)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(OPENSSL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/libeay32_debug_x64.lib CACHE INTERNAL "OpenSSL libraries." FORCE)
    else()
        set(OPENSSL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/libeay32_release_x64.lib CACHE INTERNAL "OpenSSL libraries." FORCE)
    endif()
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(OPENSSL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/libeay32_debug_win32.lib CACHE INTERNAL "OpenSSL libraries." FORCE)
    else()
        set(OPENSSL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/libeay32_release_win32.lib CACHE INTERNAL "OpenSSL libraries." FORCE)
    endif()
endif()

# set dependencies include dirs paths
set(OPENSSL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dep/openssl CACHE INTERNAL "OpenSSL include dir." FORCE)

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
    ${MYSQL_DLL}
    ${DEPENDENCY_DLLS}/libeay32.dll
)

#check for db update files
set(PATH_DB_FILES ${CMAKE_SOURCE_DIR}/sql/)

set(INSTALL_DB_FILES ${PATH_DB_FILES})


if(MSVC)
    include(${CMAKE_SOURCE_DIR}/cmake/Compilers/msvc.cmake)
else()
    message(FATAL_ERROR "Compiler is not supported")
endif()
