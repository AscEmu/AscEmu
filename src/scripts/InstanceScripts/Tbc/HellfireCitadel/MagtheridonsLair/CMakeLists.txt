# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Tbc/HellfireCitadel/MagtheridonsLair)

set(SRC_TBC_MAGTHERIDONSLAIR_SCRIPTS_FILES
    ${PATH_PREFIX}/Raid_Magtheridons_Lair.cpp
	${PATH_PREFIX}/Magtheridon.cpp
	
	${PATH_PREFIX}/Raid_Magtheridons_Lair.hpp
	${PATH_PREFIX}/Magtheridon.hpp
)

source_group(Tbc/HellfireCitadel/MagtheridonsLair FILES ${SRC_TBC_MAGTHERIDONSLAIR_SCRIPTS_FILES})
unset(PATH_PREFIX)
