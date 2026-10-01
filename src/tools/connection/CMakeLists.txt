# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME connection_patcher)

file(GLOB SRC_CONNECTION *.cpp *.hpp)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${EXECUTABLE_NAME} ${SRC_CONNECTION})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

# include library dependencies (OpenSSL for the certificate bundle of the Battle.net clients).
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    mpqlib
    ${OPENSSL_LIBRARIES}
)

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})

# the OpenSSL runtime next to the patcher
if (WIN32)
    get_filename_component(CONNECTION_OPENSSL_BIN_DIR "${OPENSSL_INCLUDE_DIR}/../bin" ABSOLUTE)
    file(GLOB CONNECTION_OPENSSL_DLLS "${CONNECTION_OPENSSL_BIN_DIR}/libcrypto-*-x64.dll")
    if (CONNECTION_OPENSSL_DLLS)
        install(FILES ${CONNECTION_OPENSSL_DLLS} DESTINATION ${ASCEMU_TOOLS_PATH})
    endif ()
    unset(CONNECTION_OPENSSL_BIN_DIR)
    unset(CONNECTION_OPENSSL_DLLS)
endif ()
