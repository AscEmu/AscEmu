project(Ascemu)

cmake_minimum_required(VERSION 2.8)
#Do system checking.
include(cmake/CompilerDefs.cmake)

SET( ROOT_PATH  ${CMAKE_SOURCE_DIR})

#We have our own custom modules that we use. This tells cmakes where to find them.
SET( CMAKE_SOURCE_DIR ${ROOT_PATH}/cmake  )
SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ROOT_PATH}/cmake )
SET( DEPENDENCY_ROOT ${ROOT_PATH}/dep )
SET( DEPENDENCY_SOURCES ${DEPENDENCY_ROOT}/src)
SET( DEPENDENCY_LIBS ${DEPENDENCY_ROOT}/lib)
SET( DEPENDENCY_INCLUDE_ROOT ${DEPENDENCY_ROOT}/include)
SET( DEPENDENCY_DLLS64 ${DEPENDENCY_ROOT}/dll64)

IF(IS_64BIT)
	SET(DEPENDENCY_DLLS ${DEPENDENCY_ROOT}/dll64)
ELSE()
	SET(DEPENDENCY_DLLS ${DEPENDENCY_ROOT}/dll)
ENDIF()

#Install libmysql.dll required for our core to run.
IF( WIN32)
	SET( INSTALLED_DEPENDENCIES ${DEPENDENCY_DLLS}/libmysql.dll ${DEPENDENCY_DLLS}/libeay32.dll)
ENDIF(WIN32)

#Our configurable variables.
include(cmake/AscConfigs.cmake)

#GetCompiler
include(cmake/CompilerVersion.cmake)

#Set generator variables
include(cmake/CompilerCMakeGenerator.cmake)

#Pass our configurable variables to the C++ preprocessor.
add_definitions( ${GLOBAL_DEFINES} -DCONFDIR=\"${ASCEMU_CONFIGSFILE_PATH}\" -DPREFIX=\"${ASCEMU_SCRIPTLIB_PATH}\")

#These are set by find_package or the zlib and pcre subprojects.
set(ZLIB_LIBRARIES "" CACHE INTERNAL "Zlib libraries.")
set(PCRE_LIBRARIES "" CACHE INTERNAL "PCRE libraries.")
set(PCRE_INCLUDE_DIRS "" CACHE INTERNAL "PCRE include dirs.")
#mark them as advanced so they don't show up on the gui.
mark_as_advanced(ZLIB_LIBRARIES PCRE_LIBRARIES PCRE_INCLUDE_DIRS)

#set global includes....
set(GLOBAL_INCLUDE_DIRS 
${ROOT_PATH}/src/shared
${ROOT_PATH}/src/world
${ROOT_PATH}/src/logonserver
${DEPENDENCY_INCLUDE_ROOT}
${DEPENDENCY_INCLUDE_ROOT}/g3dlite
${DEPENDENCY_INCLUDE_ROOT}/vmaplib
${DEPENDENCY_INCLUDE_ROOT}/pcre
${DEPENDENCY_INCLUDE_ROOT}/zlib
${DEPENDENCY_INCLUDE_ROOT}/recast
${DEPENDENCY_INCLUDE_ROOT}/detour
${DEPENDENCY_INCLUDE_ROOT}/g3dlite )

#Find our needed libs
IF( NOT WIN32 )
	find_package(ZLIB REQUIRED)
	find_package(PCRE REQUIRED)
	find_package(OpenSSL REQUIRED)
	find_package(Threads REQUIRED)
	find_package(MySQL REQUIRED)
ELSE()
	add_subdirectory(cmake/zlib)
	add_subdirectory(cmake/pcre)
	SET(MYSQL_LIBRARY "")
ENDIF()

IF(WIN32)	#On windows, we have to specify the includes for libeay and mysql libs.
	SET( GLOBAL_INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} 
	${DEPENDENCY_INCLUDE_ROOT}/openssl
	${DEPENDENCY_INCLUDE_ROOT}/mysql )
ELSE()	#On apple n linux, mysql.h should have been found.
	SET( GLOBAL_INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIR} )
ENDIF()

#For windows, link to libeay and libmysql
SET( EXTRA_LIBS)
SET( EXTRA_LIBS_PATH)
if(WIN32)
	if( IS_64BIT )
		SET( EXTRA_LIBS libeay32_win64.lib )
		if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			SET(EXTRA_LIBS ${EXTRA_LIBS} libmysql_debug_x64.lib )
		else()
			SET( EXTRA_LIBS ${EXTRA_LIBS} libmysql_release_x64.lib )
		endif()
	else(IS_64BIT )
		SET( EXTRA_LIBS libeay32_win32.lib )
		if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			SET(EXTRA_LIBS ${EXTRA_LIBS} libmysql_debug_win32.lib )
		else()
			SET( EXTRA_LIBS ${EXTRA_LIBS} libmysql_release_win32.lib )
		endif()
	endif()
	#Needed for socket stuff and crash handler
	SET(EXTRA_LIBS ${EXTRA_LIBS} ws2_32.lib dbghelp.lib )
elseif(APPLE)
	SET( EXTRA_LIBS ${EXTRA_LIBS} "-framework Carbon" )
elseif(IS_LINUX)
	SET( EXTRA_LIBS ${EXTRA_LIBS} dl )
else()
endif()

# set apple compiler flags
include(cmake/AppleCompilerFlags.cmake)

#add dependencies
add_subdirectory(cmake/g3dlite)
add_subdirectory(cmake/vmaplib)
add_subdirectory(cmake/recast)
add_subdirectory(cmake/detour)

#add dependecies
add_subdirectory(dep)

#add executables
add_subdirectory(src)

#add modules
add_subdirectory(cmake/spellhandlers)
add_subdirectory(cmake/battlegrounds)


IF(NOT WIN32)
	add_subdirectory(cmake/crashreport)
ENDIF()
add_subdirectory(cmake/scripts)


INSTALL(FILES ${INSTALLED_DEPENDENCIES} DESTINATION .)
