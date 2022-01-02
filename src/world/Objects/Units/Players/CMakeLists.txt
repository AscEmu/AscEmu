# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Objects/Units/Players)

set(SRC_UNITS_PLAYERS_FILES
    ${PATH_PREFIX}/DeathKnight.cpp
    ${PATH_PREFIX}/Player.cpp
    ${PATH_PREFIX}/Player.h
    ${PATH_PREFIX}/Player.Legacy.cpp
    ${PATH_PREFIX}/PlayerClasses.hpp
    ${PATH_PREFIX}/PlayerDefines.hpp
    ${PATH_PREFIX}/PlayerStats.cpp
    ${PATH_PREFIX}/TradeData.hpp
    ${PATH_PREFIX}/TradeData.cpp
)

source_group(Objects\\Units\\Players FILES ${SRC_UNITS_PLAYERS_FILES})
unset(PATH_PREFIX)
