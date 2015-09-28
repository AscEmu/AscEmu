# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

#set our variables
set(BUILD_EXTRAS_CREATUREDATA FALSE CACHE BOOL "Build creature data extractor")

if(BUILD_EXTRAS_CREATUREDATA)
	add_subdirectory(creature_data)
endif()