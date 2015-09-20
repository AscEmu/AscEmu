# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# add our dependecies
add_subdirectory(bzip2)
add_subdirectory(dbcfile)
add_subdirectory(loadlib)
add_subdirectory(vmaplib)
add_subdirectory(recastnavigation)
add_subdirectory(g3dlite)
add_subdirectory(libmpq_new)

# add zlib and pcre for windows compile
if(NEED_ZLIB_PCRE)
	add_subdirectory(zlib)
	add_subdirectory(pcre)
endif()

# add lualib if we need it for LuaEngin or LuaBridge
add_subdirectory(lualib)
