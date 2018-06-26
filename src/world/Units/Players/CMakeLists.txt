# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Units/Players)

set(SRC_UNITS_PLAYERS_FILES
   ${PATH_PREFIX}/DeathKnight.cpp
   ${PATH_PREFIX}/Player.cpp
   ${PATH_PREFIX}/Player.h
   ${PATH_PREFIX}/Player.Legacy.cpp
   ${PATH_PREFIX}/PlayerClasses.hpp
   ${PATH_PREFIX}/PlayerDefines.hpp
   ${PATH_PREFIX}/PlayerCache.cpp
   ${PATH_PREFIX}/PlayerCache.h
   ${PATH_PREFIX}/PlayerPacketWrapper.cpp
)

source_group(Units\\Players FILES ${SRC_UNITS_PLAYERS_FILES})
unset(PATH_PREFIX)
