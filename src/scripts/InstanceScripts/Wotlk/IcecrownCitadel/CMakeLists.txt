# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Wotlk/IcecrownCitadel)

set(SRC_WOTLK_ICECROWNCITADEL_SCRIPTS_FILES
    ${PATH_PREFIX}/Raid_IceCrownCitadel.cpp
	${PATH_PREFIX}/LordMarrowgar.cpp
	${PATH_PREFIX}/LadyDeathwhisper.cpp
	${PATH_PREFIX}/GunshipBattle.cpp
	${PATH_PREFIX}/Saurfang.cpp
	
	${PATH_PREFIX}/Raid_IceCrownCitadel.hpp
	${PATH_PREFIX}/LordMarrowgar.hpp
	${PATH_PREFIX}/LadyDeathwhisper.hpp
	${PATH_PREFIX}/GunshipBattle.hpp
	${PATH_PREFIX}/Saurfang.hpp
)

source_group(Wotlk/IcecrownCitadel FILES ${SRC_WOTLK_ICECROWNCITADEL_SCRIPTS_FILES})
unset(PATH_PREFIX)
