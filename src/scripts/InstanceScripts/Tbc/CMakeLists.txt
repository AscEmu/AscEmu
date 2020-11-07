# Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Tbc)

set(SRC_TBC_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_Arcatraz.cpp 
	${PATH_PREFIX}/Instance_AuchenaiCrypts.cpp 
	${PATH_PREFIX}/Instance_BlackMorass.cpp
	${PATH_PREFIX}/Instance_BloodFurnace.cpp
	${PATH_PREFIX}/Instance_Botanica.cpp
	${PATH_PREFIX}/Instance_HellfireRamparts.cpp
	${PATH_PREFIX}/Instance_MagistersTerrace.cpp
	${PATH_PREFIX}/Instance_ManaTombs.cpp 
	${PATH_PREFIX}/Instance_OldHillsbradFoothills.cpp
	${PATH_PREFIX}/Instance_SethekkHalls.cpp
	${PATH_PREFIX}/Instance_ShadowLabyrinth.cpp
	${PATH_PREFIX}/Instance_TheMechanar.cpp 
	${PATH_PREFIX}/Instance_TheShatteredHalls.cpp
	${PATH_PREFIX}/Instance_TheSlavePens.cpp
	${PATH_PREFIX}/Instance_TheSteamvault.cpp
	${PATH_PREFIX}/Instance_TheStockade.cpp
	${PATH_PREFIX}/Instance_TheUnderbog.cpp
	${PATH_PREFIX}/Raid_BlackTemple.cpp 
	${PATH_PREFIX}/Raid_CoT_BattleOfMountHyjal.cpp 
	${PATH_PREFIX}/Raid_GruulsLair.cpp
	${PATH_PREFIX}/Raid_Karazhan.cpp 
	${PATH_PREFIX}/Raid_Magtheridons_Lair.cpp
	${PATH_PREFIX}/Raid_SerpentshrineCavern.cpp
	${PATH_PREFIX}/Raid_SunwellPlateau.cpp
    ${PATH_PREFIX}/Raid_TheEye.cpp 
    ${PATH_PREFIX}/Raid_ZulAman.cpp 
	${PATH_PREFIX}/Instance_Arcatraz.h 
	${PATH_PREFIX}/Instance_AuchenaiCrypts.h 
	${PATH_PREFIX}/Instance_BlackMorass.h
	${PATH_PREFIX}/Instance_BloodFurnace.h
	${PATH_PREFIX}/Instance_Botanica.h
	${PATH_PREFIX}/Instance_HellfireRamparts.h
	${PATH_PREFIX}/Instance_MagistersTerrace.h
	${PATH_PREFIX}/Instance_ManaTombs.h 
	${PATH_PREFIX}/Instance_OldHillsbradFoothills.h
	${PATH_PREFIX}/Instance_SethekkHalls.h
	${PATH_PREFIX}/Instance_ShadowLabyrinth.h
	${PATH_PREFIX}/Instance_TheMechanar.h 
	${PATH_PREFIX}/Instance_TheShatteredHalls.h
	${PATH_PREFIX}/Instance_TheSlavePens.h
	${PATH_PREFIX}/Instance_TheSteamvault.h
	${PATH_PREFIX}/Instance_TheStockade.h
	${PATH_PREFIX}/Instance_TheUnderbog.h
	${PATH_PREFIX}/Raid_BlackTemple.h 
	${PATH_PREFIX}/Raid_CoT_BattleOfMountHyjal.h 
	${PATH_PREFIX}/Raid_GruulsLair.h
	${PATH_PREFIX}/Raid_Karazhan.h 
	${PATH_PREFIX}/Raid_Magtheridons_Lair.h
	${PATH_PREFIX}/Raid_SerpentshrineCavern.h
	${PATH_PREFIX}/Raid_SunwellPlateau.h
    ${PATH_PREFIX}/Raid_TheEye.h 
    ${PATH_PREFIX}/Raid_ZulAman.h 
)

source_group(Tbc FILES ${SRC_TBC_SCRIPTS_FILES})
unset(PATH_PREFIX)