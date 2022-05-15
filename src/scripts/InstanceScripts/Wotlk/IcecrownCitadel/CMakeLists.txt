# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Wotlk/IcecrownCitadel)

set(SRC_WOTLK_ICECROWNCITADEL_SCRIPTS_FILES
    ${PATH_PREFIX}/Raid_IceCrownCitadel.cpp
	${PATH_PREFIX}/LordMarrowgar.cpp
	${PATH_PREFIX}/LadyDeathwhisper.cpp
	${PATH_PREFIX}/GunshipBattle.cpp
	${PATH_PREFIX}/Saurfang.cpp
	
	${PATH_PREFIX}/Raid_IceCrownCitadel.h
	${PATH_PREFIX}/LordMarrowgar.h
	${PATH_PREFIX}/LadyDeathwhisper.h
	${PATH_PREFIX}/GunshipBattle.h
	${PATH_PREFIX}/Saurfang.h
)

source_group(Wotlk/IcecrownCitadel FILES ${SRC_WOTLK_ICECROWNCITADEL_SCRIPTS_FILES})
unset(PATH_PREFIX)
