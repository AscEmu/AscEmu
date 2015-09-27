# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(Ascemu)

# check we are using a proper cmake-version
cmake_minimum_required(VERSION 2.8)

#do system checking
include(cmake/CompilerDefs.cmake)

#set our PATH variables
set(ROOT_PATH ${CMAKE_SOURCE_DIR})
#We have our own custom modules that we use. This tells cmakes where to find them.
set(CMAKE_MODULE_PATH ${ROOT_PATH}/cmake)
set(DEPENDENCY_ROOT ${ROOT_PATH}/dep)
set(DEPENDENCY_LIBS ${DEPENDENCY_ROOT}/lib)
set(DEPENDENCY_DLLS64 ${DEPENDENCY_ROOT}/dll64)
set(RECASTNAVIGATION ${DEPENDENCY_ROOT}/recastnavigation)
set(LOADLIB_PATH ${ROOT_PATH}/src/tools/loadlib)

#check for including our dependencies
if(IS_64BIT)
   set(DEPENDENCY_DLLS ${DEPENDENCY_ROOT}/dll64)
else()
   set(DEPENDENCY_DLLS ${DEPENDENCY_ROOT}/dll)
endif()

#Install libmysql.dll required for our core to run.
if(WIN32)
   set(INSTALLED_DEPENDENCIES ${DEPENDENCY_DLLS}/libmysql.dll ${DEPENDENCY_DLLS}/libeay32.dll)
endif(WIN32)

#our config variables.
include(cmake/AscConfigs.cmake)

#getCompiler version
include(cmake/CompilerVersion.cmake)

#set generator variables
include(cmake/CompilerCMakeGenerator.cmake)

#Pass our configurable variables to the C++ preprocessor.
add_definitions(${GLOBAL_DEFINES} -DCONFDIR=\"${ASCEMU_CONFIGSFILE_PATH}\" -DPREFIX=\"${ASCEMU_SCRIPTLIB_PATH}\")

#These are set by find_package or the zlib and pcre subprojects.
set(ZLIB_LIBRARIES "" CACHE INTERNAL "Zlib libraries.")
set(PCRE_LIBRARIES "" CACHE INTERNAL "PCRE libraries.")
set(PCRE_INCLUDE_DIRS "" CACHE INTERNAL "PCRE include dirs.")
#mark them as advanced so they don't show up on the gui.
mark_as_advanced(ZLIB_LIBRARIES PCRE_LIBRARIES PCRE_INCLUDE_DIRS)

#set global includes
set(GLOBAL_INCLUDE_DIRS 
   ${ROOT_PATH}/src/shared
   ${ROOT_PATH}/src/world
   ${ROOT_PATH}/src/logonserver
   ${DEPENDENCY_INCLUDE_ROOT}
   ${DEPENDENCY_ROOT}/g3dlite/include
   ${DEPENDENCY_ROOT}/vmaplib/include
   ${DEPENDENCY_ROOT}/pcre/include
   #${DEPENDENCY_ROOT}/zlib
   ${RECASTNAVIGATION}/Recast/Include
   ${RECASTNAVIGATION}/Detour/Include
   )

#find our needed packages
if(NOT WIN32)
   find_package(ZLIB REQUIRED)
   find_package(PCRE REQUIRED)
   find_package(OpenSSL REQUIRED)
   find_package(Threads REQUIRED)
   find_package(MySQL REQUIRED)
else()
   set(NEED_ZLIB_PCRE TRUE)
endif()

#on windows, we have to specify the includes for libeay and mysql libs.
if(WIN32)
   set(GLOBAL_INCLUDE_DIRS
   ${GLOBAL_INCLUDE_DIRS} 
   ${DEPENDENCY_ROOT}/openssl
   ${DEPENDENCY_ROOT}/mysql
   )
else()  #On apple n linux, mysql.h should have been found.
   set(GLOBAL_INCLUDE_DIRS
   ${GLOBAL_INCLUDE_DIRS}
   ${MYSQL_INCLUDE_DIR}
   )
endif()

#For windows, link to libeay and libmysql
set(EXTRA_LIBS)
set(EXTRA_LIBS_PATH)
if(WIN32)
	if( IS_64BIT )
		set(EXTRA_LIBS libeay32_win64.lib)
		if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			set(EXTRA_LIBS ${EXTRA_LIBS} libmysql_debug_x64.lib)
		else()
			set(EXTRA_LIBS ${EXTRA_LIBS} libmysql_release_x64.lib)
		endif()
	else(IS_64BIT )
		set(EXTRA_LIBS libeay32_win32.lib)
		if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			set(EXTRA_LIBS ${EXTRA_LIBS} libmysql_debug_win32.lib)
		else()
			set(EXTRA_LIBS ${EXTRA_LIBS} libmysql_release_win32.lib)
		endif()
	endif()
	#Needed for socket stuff and crash handler
	set(EXTRA_LIBS ${EXTRA_LIBS} ws2_32.lib dbghelp.lib)
elseif(APPLE)
	set(EXTRA_LIBS ${EXTRA_LIBS} "-framework Carbon")
elseif(IS_LINUX)
	set(EXTRA_LIBS ${EXTRA_LIBS} dl)
else()
endif()

# set apple compiler flags
include(cmake/AppleCompilerFlags.cmake)

#add dependecies
add_subdirectory(dep)

#add executables
add_subdirectory(src)

#add modules
add_subdirectory(src/scripts)

install(FILES ${INSTALLED_DEPENDENCIES} DESTINATION .)
