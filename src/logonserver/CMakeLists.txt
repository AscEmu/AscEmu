# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(logon CXX)

include(Auth/CMakeLists.txt)
include(Console/CMakeLists.txt)
include(LogonCommServer/CMakeLists.txt)
include(Server/CMakeLists.txt)

set(SRC_LOGON_PROJECT
   ${CMAKE_CURRENT_SOURCE_DIR}/LogonStdAfx.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/LogonStdAfx.h
)

if(WIN32)
   set(SRC_LOGON_PROJECT ${SRC_LOGON_PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc)
endif()

source_group(logon FILES ${SRC_LOGON_PROJECT})

set(sources
   ${SRC_AUTH_FILES}
   ${SRC_CONSOLE_FILES}
   ${SRC_LOGON_COMM_SERVER_FILES}
   ${SRC_SERVER_FILES}
   ${SRC_LOGON_PROJECT}
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

link_directories(${EXTRA_LIBS_PATH} ${DEPENDENCY_LIBS})

add_executable(${PROJECT_NAME} ${sources})

if(USE_PCH)
    gen_cxx_pch(${PROJECT_NAME} LogonStdAfx.h ${CMAKE_CURRENT_SOURCE_DIR}/LogonStdAfx.cpp)
endif()

if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LogonConf.h.in ${CMAKE_CURRENT_SOURCE_DIR}/LogonConf.h)

#Link our shared lib.
target_link_libraries(${PROJECT_NAME} shared ${MYSQL_LIBRARIES} ${ZLIB_LIBRARIES} ${PCRE_LIBRARIES})

#Provide make install rules.
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

unset(sources)
unset(headers)
