# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# get architecture
macro(IS_ARCH_64BIT)
	if(NOT WIN32)
		if(CMAKE_SYSTEM_PROCESSOR MATCHES "[xX]64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "[xX]86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "[aA][mM][dD]64")
			set(IS_64BIT TRUE)
		else()
			set(IS_64BIT FALSE)
		endif()
		
	else(NOT WIN32)
		if(CMAKE_GENERATOR MATCHES Win64*)
			set(IS_64BIT TRUE)
		else(CMAKE_GENERATOR MATCHES Win64*)
			set(IS_64BIT FALSE)
		endif(CMAKE_GENERATOR MATCHES Win64*)
		
	endif(NOT WIN32)
	
endmacro(IS_ARCH_64BIT)

# get environment
if(WIN32)
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX "C:/Ascemu" CACHE PATH "Install path prefix" FORCE)
	endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    
    add_definitions(-DWIN32)
endif(WIN32)

set(GLOBAL_DEFINES "-DHAVE_CONFIG_H")

#set defines for msvc
if(CMAKE_GENERATOR MATCHES Visual*)
	set(GLOBAL_DEFINES ${GLOBAL_DEFINES} "/MP")
   set(GLOBAL_DEFINES ${GLOBAL_DEFINES} "/bigobj")
endif()

#check platform version.
if(IS_64BIT)
	set(GLOBAL_DEFINES ${GLOBAL_DEFINES} -"DX64")
	if(CMAKE_GENERATOR MATCHES NMake*)
		set(GLOBAL_DEFINES ${GLOBAL_DEFINES} "-bigobj")
	endif()
endif()

#mac osx
if(APPLE)
	set(GLOBAL_DEFINES ${GLOBAL_DEFINES} "-DHAVE_DARWIN")
	set(IS_MAC TRUE)
elseif(UNIX)
	#mac has unix defined too but if 'if(APPLE)' fails, then it's not apple.
	
	#check for freebsd
	if((CMAKE_SYSTEM_NAME  STREQUAL "FreeBSD") OR (CMAKE_SYSTEM_NAME STREQUAL kFreeBSD))
		set(GLOBAL_DEFINES ${GLOBAL_DEFINES} "-DUSE_KQUEUE")
		set(IS_FREEBSD TRUE)
	else()
		set(GLOBAL_DEFINES ${GLOBAL_DEFINES} "-DUSE_EPOLL")
		set(IS_LINUX TRUE)
	endif()
endif()
