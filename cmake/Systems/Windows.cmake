# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

message(STATUS "Applying settings for Windows system")

# set default install prefix if it wasn't setted up
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "C:/AscEmu" CACHE PATH "Install path prefix" FORCE)
endif ()

find_package(MySQL REQUIRED)
find_package(OpenSSL REQUIRED)

# needed for socket stuff and crash handler
set(EXTRA_LIBS
    ws2_32
    dbghelp
)

if (MSVC)
    include(${CMAKE_SOURCE_DIR}/cmake/Compilers/msvc.cmake)
else ()
    message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER_ID} is not supported")
endif ()

# check for db update files
set(PATH_DB_FILES ${CMAKE_SOURCE_DIR}/sql/)
set(INSTALL_DB_FILES ${PATH_DB_FILES})

# install libraries for windows build (libmysql.dll)
install(FILES ${MYSQL_DLL} DESTINATION .)

# install the OpenSSL runtime next to the servers (libcrypto for all, libssl for the TLS of bnetserver)
get_filename_component(OPENSSL_BIN_DIR "${OPENSSL_INCLUDE_DIR}/../bin" ABSOLUTE)
file(GLOB OPENSSL_RUNTIME_DLLS
    "${OPENSSL_BIN_DIR}/libcrypto-*-x64.dll"
    "${OPENSSL_BIN_DIR}/libssl-*-x64.dll"
)

if (OPENSSL_RUNTIME_DLLS)
    message(STATUS "Found OpenSSL dlls: ${OPENSSL_RUNTIME_DLLS}")
    install(FILES ${OPENSSL_RUNTIME_DLLS} DESTINATION .)
else ()
    message(WARNING "OpenSSL dlls not found in ${OPENSSL_BIN_DIR}; copy libcrypto and libssl next to the servers manually")
endif ()

unset(OPENSSL_BIN_DIR)
unset(OPENSSL_RUNTIME_DLLS)
