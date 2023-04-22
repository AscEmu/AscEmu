# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(logon CXX)

file(GLOB SRC_AUTH_FILES Auth/*.cpp Auth/*.h Auth/*.hpp)
file(GLOB SRC_CONSOLE_FILES Console/*.cpp Console/*.h Console/*.hpp)
file(GLOB SRC_LOGON_COMM_SERVER_FILES LogonCommServer/*.cpp LogonCommServer/*.h LogonCommServer/*.hpp)
file(GLOB SRC_REALM_FILES Realm/*.cpp Realm/*.h Realm/*.hpp)
file(GLOB SRC_SERVER_FILES Server/*.cpp Server/*.h Server/*.hpp)
file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

set(LOGON_SRCS
    ${SRC_AUTH_FILES}
    ${SRC_CONSOLE_FILES}
    ${SRC_LOGON_COMM_SERVER_FILES}
    ${SRC_REALM_FILES}
    ${SRC_SERVER_FILES}
    ${SRC_LOGON_PROJECT}
    ${SRC_LOCAL_DIR}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Opcodes
    ${CMAKE_SOURCE_DIR}/src/shared
    ${ZLIB_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${PCRE_INCLUDE_DIR}
    ${MYSQL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# source group.
source_group(logon FILES ${SRC_LOGON_PROJECT})
source_group(Auth FILES ${SRC_AUTH_FILES})
source_group(Console FILES ${SRC_CONSOLE_FILES})
source_group(LogonCommServer FILES ${SRC__LOGON_COMM_SERVER_FILES})
source_group(Realm FILES ${SRC_REALM_FILES})
source_group(Server FILES ${SRC_SERVER_FILES})

link_directories(${EXTRA_LIBS_PATH} ${DEPENDENCY_LIBS})

add_executable(${PROJECT_NAME} ${LOGON_SRCS})

if (APPLE)
    target_link_libraries(${PROJECT_NAME} c++)
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
    target_link_libraries(${PROJECT_NAME} c++experimental)
endif ()

if (WIN32)
    set(SRC_LOGON_PROJECT ${SRC_LOGON_PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LogonConf.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/LogonConf.hpp)

# link our shared lib.
target_link_libraries(${PROJECT_NAME} shared ${MYSQL_LIBRARIES} ${ZLIB_LIBRARIES} ${PCRE_LIBRARIES})

# provide make install rules.
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

# generate precompiled header.
if (USE_PCH)
    message(STATUS "Generate PCH for ${PROJECT_NAME}")
    target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchLogon.hpp)
endif ()
