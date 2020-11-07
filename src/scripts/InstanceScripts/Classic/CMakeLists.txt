# Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Classic)

set(SRC_CLASSIC_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_BlackfathomDeeps.cpp
	${PATH_PREFIX}/Instance_BlackrockDepths.cpp
	${PATH_PREFIX}/Instance_BlackrockSpire.cpp
	${PATH_PREFIX}/Instance_Deadmines.cpp
	${PATH_PREFIX}/Instance_DireMaul.cpp
	${PATH_PREFIX}/Instance_Gnomeregan.cpp
	${PATH_PREFIX}/Instance_Maraudon.cpp
	${PATH_PREFIX}/Instance_RagefireChasm.cpp
	${PATH_PREFIX}/Instance_RazorfenDowns.cpp
	${PATH_PREFIX}/Instance_RazorfenKraul.cpp
	${PATH_PREFIX}/Instance_ScarletMonastery.cpp
	${PATH_PREFIX}/Instance_Scholomance.cpp
	${PATH_PREFIX}/Instance_ShadowfangKeep.cpp
	${PATH_PREFIX}/Instance_Stratholme.cpp
	${PATH_PREFIX}/Instance_TheTempleOfAtalHakkar.cpp
	${PATH_PREFIX}/Instance_Uldaman.cpp
	${PATH_PREFIX}/Instance_WailingCaverns.cpp 
	${PATH_PREFIX}/Instance_ZulFarrak.cpp
	${PATH_PREFIX}/Raid_BlackwingLair.cpp
	${PATH_PREFIX}/Raid_MoltenCore.cpp
	${PATH_PREFIX}/Raid_OnyxiasLair.cpp
	${PATH_PREFIX}/Raid_ZulGurub.cpp
	${PATH_PREFIX}/Raid_WorldBosses.cpp
	${PATH_PREFIX}/Instance_BlackfathomDeeps.h
	${PATH_PREFIX}/Instance_BlackrockDepths.h
	${PATH_PREFIX}/Instance_BlackrockSpire.h
	${PATH_PREFIX}/Instance_Deadmines.h
	${PATH_PREFIX}/Instance_DireMaul.h
	${PATH_PREFIX}/Instance_Gnomeregan.h
	${PATH_PREFIX}/Instance_Maraudon.h
	${PATH_PREFIX}/Instance_RagefireChasm.h
	${PATH_PREFIX}/Instance_RazorfenDowns.h
	${PATH_PREFIX}/Instance_RazorfenKraul.h
	${PATH_PREFIX}/Instance_ScarletMonastery.h
	${PATH_PREFIX}/Instance_Scholomance.h
	${PATH_PREFIX}/Instance_ShadowfangKeep.h
	${PATH_PREFIX}/Instance_Stratholme.h
	${PATH_PREFIX}/Instance_TheTempleOfAtalHakkar.h
	${PATH_PREFIX}/Instance_Uldaman.h
	${PATH_PREFIX}/Instance_WailingCaverns.h 
	${PATH_PREFIX}/Instance_ZulFarrak.h
	${PATH_PREFIX}/Raid_BlackwingLair.h
	${PATH_PREFIX}/Raid_MoltenCore.h
	${PATH_PREFIX}/Raid_OnyxiasLair.h
	${PATH_PREFIX}/Raid_ZulGurub.h
	${PATH_PREFIX}/Raid_WorldBosses.h
)

source_group(Classic FILES ${SRC_CLASSIC_SCRIPTS_FILES})
unset(PATH_PREFIX)