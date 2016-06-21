# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

#ignore the warnings for dependencies (external code)
if(MSVC)
    add_definitions(/W0)
else()
    add_definitions(-w)
endif()

# add zlib and pcre for windows compile
if(WIN32)
    add_subdirectory(bzip2)
	add_subdirectory(zlib)
	add_subdirectory(pcre)
endif()

# add our dependecies
add_subdirectory(recastnavigation)
add_subdirectory(g3dlite)

if(BUILD_TOOLS)
    add_subdirectory(libmpq)
    add_subdirectory(dbcfile)
    add_subdirectory(loadlib)
endif()

# add lualib if we need it for LuaEngine
if(BUILD_LUAENGINE)
    add_subdirectory(lualib)
endif()
