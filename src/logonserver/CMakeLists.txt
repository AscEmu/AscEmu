# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(logon CXX)

include(LogonCommServer/CMakeLists.txt)

set(sources
   ${SRC_LOGON_COMM_SERVER_FILES}
   AccountCache.cpp
   AuthSocket.cpp
   AutoPatcher.cpp
   LogonConsole.cpp
   LogonStdAfx.cpp
   Main.cpp
   Master.cpp
   )

set(headers
	AccountCache.h
	AuthSocket.h
	AuthStructs.h
	AutoPatcher.h
	LogonConsole.h
	LogonStdAfx.h
    Master.hpp
    LogonServerDefines.hpp
	PeriodicFunctionCall_Thread.h
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

if(WIN32)
    set(sources ${sources} resources.rc)
endif()

add_executable(${PROJECT_NAME} ${sources} ${headers})

if(USE_PCH AND PCHSupport_FOUND)
    ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} LogonStdAfx.h ${CMAKE_CURRENT_SOURCE_DIR}/LogonStdAfx.cpp)
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LogonConf.h.in ${CMAKE_CURRENT_SOURCE_DIR}/LogonConf.h)

#Link our shared lib.
target_link_libraries(${PROJECT_NAME} shared ${MYSQL_LIBRARIES} ${ZLIB_LIBRARIES} ${PCRE_LIBRARIES})

#Provide make install rules.
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

unset(sources)
unset(headers)
