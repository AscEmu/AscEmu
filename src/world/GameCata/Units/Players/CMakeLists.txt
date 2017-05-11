# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameCata/Units/Players)

set(SRC_GAMECATA_UNITS_PLAYERS_FILES
    ${PATH_PREFIX}/Player.Cata.cpp
    ${PATH_PREFIX}/Player.Auction.Cata.cpp
    ${PATH_PREFIX}/Player.Spells.Cata.cpp
    ${PATH_PREFIX}/Player.Trade.Cata.cpp
)

source_group(GameCata\\Units\\Players FILES ${SRC_GAMECATA_UNITS_PLAYERS_FILES})
unset(PATH_PREFIX)
