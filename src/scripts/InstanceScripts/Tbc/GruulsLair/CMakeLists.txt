# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Tbc/GruulsLair)

set(SRC_TBC_GRUULSLAIR_SCRIPTS_FILES
    ${PATH_PREFIX}/Raid_GruulsLair.cpp
	${PATH_PREFIX}/GruulTheDragonKiller.cpp
	${PATH_PREFIX}/HighKingMaulgar.cpp
	
	${PATH_PREFIX}/Raid_GruulsLair.hpp
	${PATH_PREFIX}/GruulTheDragonKiller.hpp
	${PATH_PREFIX}/HighKingMaulgar.hpp
)

source_group(Tbc/GruulsLair FILES ${SRC_TBC_GRUULSLAIR_SCRIPTS_FILES})
unset(PATH_PREFIX)
