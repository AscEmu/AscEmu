# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME logon)

file(GLOB SRC_AUTH_FILES Auth/*.cpp Auth/*.h Auth/*.hpp)
file(GLOB SRC_CONSOLE_FILES Console/*.cpp Console/*.h Console/*.hpp)
file(GLOB SRC_LOGON_COMM_SERVER_FILES LogonCommServer/*.cpp LogonCommServer/*.h LogonCommServer/*.hpp)
file(GLOB SRC_REALM_FILES Realm/*.cpp Realm/*.h Realm/*.hpp)
file(GLOB SRC_SERVER_FILES Server/*.cpp Server/*.h Server/*.hpp)
file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

if (WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
endif ()

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${EXECUTABLE_NAME}
    ${SRC_AUTH_FILES}
    ${SRC_CONSOLE_FILES}
    ${SRC_LOGON_COMM_SERVER_FILES}
    ${SRC_REALM_FILES}
    ${SRC_SERVER_FILES}
    ${SRC_LOCAL_DIR}
    ${APP_ICON_RESOURCE_WINDOWS}
)

target_include_directories(${EXECUTABLE_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/shared
    ${ZLIB_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
    shared
    ${MYSQL_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LogonConf.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/LogonConf.hpp)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generate PCH for ${EXECUTABLE_NAME}")
    target_precompile_headers(${EXECUTABLE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchLogon.hpp)
endif ()

# provide make install rules.
install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION .)
