# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up project name
project(lualib C)

# set our prefix (used to link files/project)
set(prefix ${DEPENDENCY_ROOT}/${PROJECT_NAME})

set(sources
   lapi.c
   lauxlib.c
   lbaselib.c
   lbitlib.c
   lcode.c
   lcorolib.c
   lctype.c
   ldblib.c
   ldebug.c
   ldo.c
   ldump.c
   lfunc.c
   lgc.c
   linit.c
   liolib.c
   llex.c
   lmathlib.c
   lmem.c
   loadlib.c
   lobject.c
   lopcodes.c
   loslib.c
   lparser.c
   lstate.c
   lstring.c
   lstrlib.c
   ltable.c
   ltablib.c
   ltm.c
   lua.c
   lundump.c
   lutf8lib.c
   lvm.c
   lzio.c
   )

set(headers
   lapi.h
   lauxlib.h
   lcode.h
   lctype.h
   ldebug.h
   ldo.h
   lfunc.h
   lgc.h
   llex.h
   llimits.h
   lmem.h
   lobject.h
   lopcodes.h
   lparser.h
   lprefix.h
   lstate.h
   lstring.h
   ltable.h
   ltm.h
   lua.h
   lua.hpp
   luaconf.h
   lualib.h
   lundump.h
   lvm.h
   lzio.h
   )

foreach(src IN ITEMS ${sources})
  set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)
foreach(src IN ITEMS ${headers})
  set(HEADERS ${HEADERS} ${prefix}/${src})
endforeach(src)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
	add_definitions( -DLUA_USE_APICHECK )
endif()

add_library(${PROJECT_NAME} STATIC ${SRCS} ${HEADERS})

remove_definitions(-std=c++0x)

if(IS_64BIT AND NOT MSVC)
   set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY COMPILE_FLAGS "-fPIC")
endif()

target_link_libraries(${PROJECT_NAME})
