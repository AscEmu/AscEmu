if(NOT BUILD_INSTANCESCRIPTS )
  message("\tInstanceScripts was skipped because BUILD_INSTANCESCRIPTS is unchecked.")
  return()
endif(NOT BUILD_INSTANCESCRIPTS)

PROJECT(InstanceScripts)
SET(prefix ${SCRIPTS_PATH}/InstanceScripts)

set(sources
    Instance_Arcatraz.cpp 
    Instance_AuchenaiCrypts.cpp 
    Instance_AzjolNerub.cpp 
    Instance_BlackfathomDeeps.cpp 
    Instance_BlackMorass.cpp 
    Instance_BlackrockDepths.cpp 
    Instance_BlackrockSpire.cpp 
    Instance_BloodFurnace.cpp 
    Instance_Botanica.cpp 
    Instance_CullingOfStratholme.cpp 
    Instance_Deadmines.cpp 
    Instance_DireMaul.cpp 
    Instance_DrakTharonKeep.cpp 
    Instance_Gundrak.cpp 
    Instance_HallsOfLightning.cpp 
    Instance_HallsOfReflection.cpp 
    Instance_HallsOfStone.cpp 
    Instance_HellfireRamparts.cpp 
    Instance_MagistersTerrace.cpp 
    Instance_ManaTombs.cpp 
    Instance_Mauradon.cpp 
    Instance_Nexus.cpp 
    Instance_OldHillsbradFoothills.cpp 
    Instance_RagefireChasm.cpp 
    Instance_RazorfenDowns.cpp 
    Instance_ScarletMonastery.cpp 
    Instance_Scholomance.cpp 
    Instance_SethekkHalls.cpp 
    Instance_ShadowLabyrinth.cpp 
    Instance_ShadowfangKeep.cpp 
    Instance_TheMechanar.cpp 
    Instance_TheShatteredHalls.cpp 
    Instance_TheSlavePens.cpp 
    Instance_TheSteamvault.cpp 
    Instance_TheUnderbog.cpp 
    Instance_TheVioletHold.cpp 
    Instance_Uldaman.cpp 
    Instance_UtgardeKeep.cpp 
    Instance_WailingCaverns.cpp 
    Instance_TheStockade.cpp 
    Instance_ZulFarrak.cpp 
    Raid_BlackTemple.cpp 
    Raid_BlackwingLair.cpp 
    Raid_CoT_BattleOfMountHyjal.cpp 
    Raid_GruulsLair.cpp
    Raid_IceCrownCitadel.cpp
    Raid_Karazhan.cpp 
    Raid_Magtheridons_Lair.cpp 
    Raid_MoltenCore.cpp 
    Raid_Naxxramas.cpp 
    Raid_OnyxiasLair.cpp 
    Raid_SerpentshrineCavern.cpp
    Raid_SunwellPlateau.cpp  
    Raid_TheEye.cpp 
    Raid_TheObsidianSanctum.cpp 
    Raid_Ulduar.cpp 
    Raid_ZulGurub.cpp 
    Raid_WorldBosses.cpp 
    Raid_ZulAman.cpp 
    ../Common/Instance_Base.cpp 
    ../Common/Base.cpp 
    Setup.cpp
    )

SET(headers
    Instance_Arcatraz.h 
    Instance_AuchenaiCrypts.h 
    Instance_AzjolNerub.h 
    Instance_BlackfathomDeeps.h 
    Instance_BlackMorass.h 
    Instance_BlackrockDepths.h 
    Instance_BlackrockSpire.h 
    Instance_BloodFurnace.h 
    Instance_Botanica.h 
    Instance_CullingOfStratholme.h 
    Instance_Deadmines.h 
    Instance_DireMaul.h 
    Instance_DrakTharonKeep.h 
    Instance_Gundrak.h 
    Instance_HallsOfLightning.h 
    Instance_HallsOfReflection.h 
    Instance_HallsOfStone.h 
    Instance_HellfireRamparts.h 
    Instance_MagistersTerrace.h 
    Instance_ManaTombs.h 
    Instance_Mauradon.h 
    Instance_Nexus.h 
    Instance_OldHillsbradFoothills.h 
    Instance_RagefireChasm.h 
    Instance_RazorfenDowns.h 
    Instance_ScarletMonastery.h 
    Instance_Scholomance.h 
    Instance_SethekkHalls.h 
    Instance_ShadowLabyrinth.h 
    Instance_ShadowfangKeep.h 
    Instance_TheMechanar.h 
    Instance_TheShatteredHalls.h 
    Instance_TheSlavePens.h 
    Instance_TheSteamvault.h 
    Instance_TheUnderbog.h 
    Instance_TheVioletHold.h 
    Instance_Uldaman.h 
    Instance_UtgardeKeep.h 
    Instance_WailingCaverns.h 
    Instance_TheStockade.h 
    Instance_ZulFarrak.h
    Raid_Naxxramas.h
    Raid_Ulduar.h
    Raid_IceCrownCitadel.h
    Setup.h
    ../Common/Base.h
    ../Common/EasyFunctions.h
    ../Common/Instance_Base.h
    )

foreach(src IN ITEMS ${sources})
  SET(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)
foreach(src IN ITEMS ${headers})
  SET(HEADERS ${HEADERS} ${prefix}/${src})
endforeach(src)

include_directories(${include_dirs} ${prefix})
add_library(${PROJECT_NAME} SHARED ${SRCS} ${HEADERS})
add_dependencies(${PROJECT_NAME} ${dependancies})
target_link_libraries(${PROJECT_NAME} ${linkages})

include(PCHSupport)
IF(CMAKE_GENERATOR MATCHES Visual*)
	ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h ${prefix}/Setup.cpp)
ENDIF()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
#Cleap up our variables(Not required.)
unset(prefix)
unset(SRCS)
unset(sources)