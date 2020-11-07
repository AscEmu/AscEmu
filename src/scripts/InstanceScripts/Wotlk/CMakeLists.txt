# Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Wotlk)

set(SRC_WOTLK_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_AhnKahetTheOldKingdom.cpp
	${PATH_PREFIX}/Instance_AzjolNerub.cpp
	${PATH_PREFIX}/Instance_CullingOfStratholme.cpp
	${PATH_PREFIX}/Instance_DrakTharonKeep.cpp
	${PATH_PREFIX}/Instance_EyeOfEternity.cpp
	${PATH_PREFIX}/Instance_ForgeOfSouls.cpp
	${PATH_PREFIX}/Instance_Gundrak.cpp
	${PATH_PREFIX}/Instance_HallsOfLightning.cpp
	${PATH_PREFIX}/Instance_HallsOfReflection.cpp
	${PATH_PREFIX}/Instance_HallsOfStone.cpp 
	${PATH_PREFIX}/Instance_Nexus.cpp
	${PATH_PREFIX}/Instance_PitOfSaron.cpp
	${PATH_PREFIX}/Instance_TheOculus.cpp
	${PATH_PREFIX}/Instance_TheVioletHold.cpp
	${PATH_PREFIX}/Instance_TrialOfTheChampion.cpp
	${PATH_PREFIX}/Instance_UtgardeKeep.cpp
	${PATH_PREFIX}/Instance_UtgardePinnacle.cpp
	${PATH_PREFIX}/Instance_VaultOfArchavon.cpp
	${PATH_PREFIX}/Raid_IceCrownCitadel.cpp
    ${PATH_PREFIX}/Raid_Naxxramas.cpp
    ${PATH_PREFIX}/Raid_TheObsidianSanctum.cpp  
    ${PATH_PREFIX}/Raid_Ulduar.cpp    
	${PATH_PREFIX}/Raid_TrialOfTheCrusader.cpp
    ${PATH_PREFIX}/Instance_AhnKahetTheOldKingdom.h
	${PATH_PREFIX}/Instance_AzjolNerub.h
	${PATH_PREFIX}/Instance_CullingOfStratholme.h
	${PATH_PREFIX}/Instance_DrakTharonKeep.h
	${PATH_PREFIX}/Instance_EyeOfEternity.h
	${PATH_PREFIX}/Instance_ForgeOfSouls.h
	${PATH_PREFIX}/Instance_Gundrak.h
	${PATH_PREFIX}/Instance_HallsOfLightning.h
	${PATH_PREFIX}/Instance_HallsOfReflection.h
	${PATH_PREFIX}/Instance_HallsOfStone.h 
	${PATH_PREFIX}/Instance_Nexus.h
	${PATH_PREFIX}/Instance_PitOfSaron.h
	${PATH_PREFIX}/Instance_TheOculus.h
	${PATH_PREFIX}/Instance_TheVioletHold.h
	${PATH_PREFIX}/Instance_TrialOfTheChampion.h
	${PATH_PREFIX}/Instance_UtgardeKeep.h
	${PATH_PREFIX}/Instance_UtgardePinnacle.h
	${PATH_PREFIX}/Instance_VaultOfArchavon.h
	${PATH_PREFIX}/Raid_IceCrownCitadel.h
    ${PATH_PREFIX}/Raid_Naxxramas.h
    ${PATH_PREFIX}/Raid_TheObsidianSanctum.h  
    ${PATH_PREFIX}/Raid_Ulduar.h 	
	${PATH_PREFIX}/Raid_TrialOfTheCrusader.h
)

source_group(Wotlk FILES ${SRC_WOTLK_SCRIPTS_FILES})
unset(PATH_PREFIX)