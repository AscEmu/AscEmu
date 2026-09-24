# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

set(EXECUTABLE_NAME bnetserver)

file(GLOB SRC_SERVER_FILES Server/*.cpp Server/*.h Server/*.hpp)
file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp *.rc)

ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${EXECUTABLE_NAME}
    ${SRC_SERVER_FILES}
    ${SRC_LOCAL_DIR}
)

target_include_directories(${EXECUTABLE_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/shared
    ${OPENSSL_INCLUDE_DIR}
)

# BNet realm-list responses use Blizzard's zlib-compressed JSON blob format.
if (TARGET zlibstatic)
    set(BNET_ZLIB_TARGET zlibstatic)
elseif (TARGET zlib)
    set(BNET_ZLIB_TARGET zlib)
elseif (TARGET ZLIB::ZLIB)
    set(BNET_ZLIB_TARGET ZLIB::ZLIB)
else ()
    message(FATAL_ERROR "bnetserver requires a zlib target")
endif ()

target_link_libraries(${EXECUTABLE_NAME}
    shared
    ${OPENSSL_LIBRARIES}
    ${BNET_ZLIB_TARGET}
)

unset(BNET_ZLIB_TARGET)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BNetConf.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/BNetConf.hpp)

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION .)
