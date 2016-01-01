# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(logon CXX)

set(prefix ${ROOT_PATH}/src/logonserver)
set(sources
   AccountCache.cpp
   AuthSocket.cpp
   AutoPatcher.cpp
   LogonCommServer.cpp
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
	LogonCommServer.h
	LogonConsole.h
	LogonStdAfx.h
   Master.hpp
   LogonServerDefines.hpp
	PeriodicFunctionCall_Thread.h
   Opcodes/LogonRealmOpcodes.hpp
   )

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)
foreach(src IN ITEMS ${headers})
   set(HEADERS ${HEADERS} ${prefix}/${src})
endforeach(src)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/Opcodes
   ${ROOT_PATH}/src/shared
   ${DEPENDENCY_ROOT}/zlib
   ${DEPENDENCY_ROOT}/openssl
   ${DEPENDENCY_ROOT}/pcre/include
   )
link_directories(${EXTRA_LIBS_PATH} ${DEPENDENCY_LIBS})

if(IS_64BIT)
   link_directories(${DEPENDENCY_DLLS64})
endif()

include(PCHSupport)

set(WIN_RESOURCE_FILE ${ROOT_PATH}/src/logonserver/resources.rc)
add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS} ${WIN_RESOURCE_FILE})

if(CMAKE_GENERATOR MATCHES Visual*)
   ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} LogonStdAfx.h ${prefix}/LogonStdAfx.cpp)
endif()

#Link our shared lib.
target_link_libraries(${PROJECT_NAME} shared ${ZLIB_LIBRARIES} ${PCRE_LIBRARIES})

#Provide make install rules.
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
