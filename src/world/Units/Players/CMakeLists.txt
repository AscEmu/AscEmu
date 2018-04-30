# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Units/Players)

set(SRC_UNITS_PLAYERS_FILES
   ${PATH_PREFIX}/DeathKnight.cpp
   ${PATH_PREFIX}/Player.cpp
   ${PATH_PREFIX}/Player.h
   ${PATH_PREFIX}/Player.Legacy.cpp
   ${PATH_PREFIX}/PlayerClasses.hpp
   ${PATH_PREFIX}/PlayerDefines.hpp
)

source_group(Units\\Players FILES ${SRC_UNITS_PLAYERS_FILES})
unset(PATH_PREFIX)
